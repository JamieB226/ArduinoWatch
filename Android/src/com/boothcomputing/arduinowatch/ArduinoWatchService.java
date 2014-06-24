package com.boothcomputing.arduinowatch;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.Set;
import java.util.TimeZone;
import java.util.Timer;
import java.util.TimerTask;
import java.util.UUID;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothAdapter.LeScanCallback;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.IBinder;
import android.os.ParcelUuid;
import android.preference.PreferenceManager;
import android.util.Log;

public class ArduinoWatchService extends Service
{
	public static String strRSSFeedURL;
	public static String strRSSFeedUserName;
	public static String strRSSFeedPassword;
	public static String strGmailUserName;
	public static String strGmailPassword;
	public static String strAOLUserName;
	public static String strAOLPassword;
	public static String strWatchBTID;
	public static int intGmailMessages;
	public static int intIMAPMessages;
	public static int intRSSUnread;
	byte[] watchBuffer = new byte[32];
	byte[] outBuffer = new byte[32];
	private static boolean blnSync;

	// UUIDs for UAT service and associated characteristics.
	public static UUID UART_UUID = UUID
			.fromString("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
	public static UUID TX_UUID = UUID
			.fromString("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
	public static UUID RX_UUID = UUID
			.fromString("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");
	// UUID for the BTLE client characteristic which is necessary for
	// notifications.
	public static UUID CLIENT_UUID = UUID
			.fromString("00002902-0000-1000-8000-00805f9b34fb");

	// BTLE state
	private BluetoothAdapter adapter;
	private BluetoothGatt gatt;
	private BluetoothGattCharacteristic tx;
	private BluetoothGattCharacteristic rx;

	final Handler timerHandler = new Handler();
	Runnable timerRunnable = new Runnable()
	{

		@Override public void run()
		{

			if(tx != null)
			{
				sendTimeUpdate();
				sendNotificationUpdate();
			}

			timerHandler.postDelayed(this, 5000);
		}
	};

	public ArduinoWatchService()
	{
		super();

		System.out.println("Service:  Created Service");
		// TODO Auto-generated constructor stub
	}

	protected void onHandleIntent(Intent intent)
	{
		System.out.println("Service:  Processing intent");

		switch (intent.getAction())
		{
		case "StartSync":
			System.out.println("Service:  StartSync");
			blnSync = true;
			break;
		case "ReadSettings":
			System.out.println("Service: ReadSettings");
			readSettings();
			break;
		case "StopSync":
			System.out.println("Service:  StopSync");
			blnSync = false;
			break;
		default:
			System.out.println("Service:  Received unknown Intent Action:"
					+ intent.getAction());
			break;
		}
	}

	public void main(String[] args)
	{

	}

	private void sendTimeUpdate()
	{
		Calendar cal = Calendar.getInstance();
		TimeZone tz = cal.getTimeZone();

		/* date formatter in local timezone */
		SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMHHmmss");
		sdf.setTimeZone(tz);

		/* Generate time string to send to watch */
		String localTime = "$tm" + sdf.format(new Date());

		try
		{
			sendToWatch(localTime);
		}
		catch (IOException e)
		{
			System.out.println("Connection to watch broken.  Command not sent:"
					+ localTime);
		}
	}

	private void readSettings()
	{
		SharedPreferences settings = getBaseContext().getSharedPreferences(
				"ArduinoWatch", Context.MODE_PRIVATE);
		strRSSFeedURL = settings.getString("RSSFeedURL", "");
		strRSSFeedUserName = settings.getString("RSSFeedUserName", "");
		strRSSFeedPassword = settings.getString("RSSFeedPassword", "");
		strGmailUserName = settings.getString("GmailUserName", "");
		strGmailPassword = settings.getString("GmailPassword", "");
		strAOLUserName = settings.getString("AOLUserName", "");
		strAOLPassword = settings.getString("AOLPassword", "");
		strWatchBTID = settings.getString("WatchBTID", "");
	}

	private void initWatchConnection() throws IOException
	{
		adapter = BluetoothAdapter.getDefaultAdapter();
		System.out.println("Scanning for devices...");
		if (adapter != null)
		{
			System.out.println("Service:  Got Default Adapter");
			if (adapter.isEnabled())
			{
				System.out.println("Service:  Bluetooth is Enabled");
				if (BluetoothAdapter.checkBluetoothAddress(strWatchBTID) == true)
				{
					System.out.println("Service:  Watch Bluetooth Address is valid.");
					adapter.startLeScan(scanCallback);
				}
				else
				{
					Log.e("error", "Watch Bluetooth Address is invalid.");
				}
			}
			else
			{
				Log.e("error", "Bluetooth is disabled.");
			}
		}
	}

	public void sendToWatch(String s) throws IOException
	{
		if (tx == null)
		{
			// Do nothing if there is no device or message to send.
			return;
		}
		// Update TX characteristic value. Note the setValue overload that
		// takes a byte array must be used.
		tx.setValue(s.getBytes());
		if (gatt.writeCharacteristic(tx))
		{
			System.out.println("Sent: " + s.getBytes());
		}
		else
		{
			System.out.println("Couldn't write TX characteristic!");
		}
	}

	private void sendNotificationUpdate()
	{
		try
		{
			sendToWatch("$nf" + Integer.toString(intGmailMessages) + ","
					+ Integer.toString(intIMAPMessages) + ","
					+ Integer.toString(intRSSUnread));
		}
		catch (IOException e)
		{
			System.out.println("Connection to watch broken.  Command not sent:"
					+ "$nf"
					+ Integer.toString(intGmailMessages)
					+ ","
					+ Integer.toString(intIMAPMessages)
					+ ","
					+ Integer.toString(intRSSUnread));
		}
	}

	@Override public IBinder onBind(Intent intent)
	{
		return null;
	}

	@Override public void onCreate()
	{
		System.out.println("Service:  Created");
	}

	@Override public void onDestroy()
	{
		System.out.println("Service:  Destroyed");
	}

	@Override public int onStartCommand(Intent intent, int flags, int startId)
	{
		readSettings();
		try
		{
			initWatchConnection();
		}
		catch (IOException e)
		{
			System.out.println("Error Connecting");
		}
		final Runnable r = new Runnable()
		{
			public void run()
			{
				timerHandler.postDelayed(timerRunnable, 5000);
				while (true)
				{
					// do stuff
				}
			}
		};
		Thread t = new Thread(r);
		t.start();
		return Service.START_STICKY;
	}

	private BluetoothGattCallback callback = new BluetoothGattCallback()
	{
		// Called whenever the device connection state changes, i.e. from
		// disconnected to connected.
		@Override public void onConnectionStateChange(BluetoothGatt gatt,
				int status, int newState)
		{
			super.onConnectionStateChange(gatt, status, newState);
			if (newState == BluetoothGatt.STATE_CONNECTED)
			{
				System.out.println("Connected!");
				// Discover services.
				if (!gatt.discoverServices())
				{
					System.out.println("Failed to start discovering services!");
				}
			}
			else if (newState == BluetoothGatt.STATE_DISCONNECTED)
			{
				System.out.println("Disconnected!");
			}
			else
			{
				System.out.println("Connection state changed. New state: "
						+ newState);
			}
		}

		// Called when services have been discovered on the remote device.
		// It seems to be necessary to wait for this discovery to occur before
		// manipulating any services or characteristics.
		@Override public void onServicesDiscovered(BluetoothGatt gatt,
				int status)
		{
			super.onServicesDiscovered(gatt, status);
			if (status == BluetoothGatt.GATT_SUCCESS)
			{
				System.out.println("Service discovery completed!");
			}
			else
			{
				System.out.println("Service discovery failed with status: "
						+ status);
			}
			// Save reference to each characteristic.
			tx = gatt.getService(UART_UUID).getCharacteristic(TX_UUID);
			rx = gatt.getService(UART_UUID).getCharacteristic(RX_UUID);
			// Setup notifications on RX characteristic changes (i.e. data
			// received).
			// First call setCharacteristicNotification to enable
			// notification.
			if (!gatt.setCharacteristicNotification(rx, true))
			{
				System.out.println("Couldn't set notifications for RX characteristic!");
			}
			// Next update the RX characteristic's client descriptor to
			// enable notifications.
			if (rx.getDescriptor(CLIENT_UUID) != null)
			{
				BluetoothGattDescriptor desc = rx
						.getDescriptor(CLIENT_UUID);
				desc.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
				if (!gatt.writeDescriptor(desc))
				{
					System.out.println("Couldn't write RX client descriptor value!");
				}
			}
			else
			{
				System.out.println("Couldn't get RX client descriptor!");
			}
		}

		// Called when a remote characteristic changes (like the RX
		// characteristic).
		@Override public void onCharacteristicChanged(BluetoothGatt gatt,
				BluetoothGattCharacteristic characteristic)
		{
			super.onCharacteristicChanged(gatt, characteristic);
			System.out.println("Received: "
					+ characteristic.getStringValue(0));
		}
	};

	// BTLE device scanning callback.
	private LeScanCallback scanCallback = new LeScanCallback()
	{
		// Called when a device is found.
		@Override public void onLeScan(BluetoothDevice bluetoothDevice,
				int i, byte[] bytes)
		{
			System.out.println("Found device: "
					+ bluetoothDevice.getAddress());
			if (bluetoothDevice.getAddress() == strWatchBTID)
			{
				// Check if the device has the UART service.
				if (parseUUIDs(bytes).contains(UART_UUID))
				{
					// Found a device, stop the scan.
					adapter.stopLeScan(scanCallback);
					System.out.println("Found UART service!");
					// Connect to the device.
					// Control flow will now go to the callback functions
					// when
					// BTLE events occur.
					gatt = bluetoothDevice.connectGatt(
							getApplicationContext(), false, callback);
				}
			}
		}
	};

	private List<UUID> parseUUIDs(final byte[] advertisedData)
	{
		List<UUID> uuids = new ArrayList<UUID>();

		int offset = 0;
		while (offset < (advertisedData.length - 2))
		{
			int len = advertisedData[offset++];
			if (len == 0)
				break;

			int type = advertisedData[offset++];
			switch (type)
			{
			case 0x02: // Partial list of 16-bit UUIDs
			case 0x03: // Complete list of 16-bit UUIDs
				while (len > 1)
				{
					int uuid16 = advertisedData[offset++];
					uuid16 += (advertisedData[offset++] << 8);
					len -= 2;
					uuids.add(UUID.fromString(String.format(
							"%08x-0000-1000-8000-00805f9b34fb", uuid16)));
				}
				break;
			case 0x06:// Partial list of 128-bit UUIDs
			case 0x07:// Complete list of 128-bit UUIDs
				// Loop through the advertised 128-bit UUID's.
				while (len >= 16)
				{
					try
					{
						// Wrap the advertised bits and order them.
						ByteBuffer buffer = ByteBuffer.wrap(
								advertisedData, offset++, 16).order(
								ByteOrder.LITTLE_ENDIAN);
						long mostSignificantBit = buffer.getLong();
						long leastSignificantBit = buffer.getLong();
						uuids.add(new UUID(leastSignificantBit,
								mostSignificantBit));
					}
					catch (IndexOutOfBoundsException e)
					{
						// Defensive programming.
						// Log.e(LOG_TAG, e.toString());
						continue;
					}
					finally
					{
						// Move the offset to read the next uuid.
						offset += 15;
						len -= 16;
					}
				}
				break;
			default:
				offset += (len - 1);
				break;
			}
		}
		return uuids;
	}
}
