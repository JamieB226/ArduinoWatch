package com.boothcomputing.arduinowatch;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
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

import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.params.BasicHttpParams;
import org.json.JSONException;
import org.json.JSONObject;

import com.firebase.client.DataSnapshot;
import com.firebase.client.Firebase;
import com.firebase.client.FirebaseError;
import com.firebase.client.ValueEventListener;

import android.accounts.Account;
import android.accounts.AccountManager;
import android.accounts.AccountManagerCallback;
import android.accounts.AccountManagerFuture;
import android.accounts.AuthenticatorException;
import android.accounts.OperationCanceledException;
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
import android.database.Cursor;
import android.os.Handler;
import android.os.IBinder;
import android.os.ParcelUuid;
import android.preference.PreferenceManager;
import android.util.Log;
import android.widget.EditText;

public class ArduinoWatchService extends Service
{
	public static String strRSSFeedURL;
	public static String strRSSFeedUserName;
	public static String strRSSFeedPassword;
	public static String strNestUserName;
	public static String strNestPIN;
	public static String strAOLUserName;
	public static String strAOLPassword;
	public static String strWatchBTID;
	public static String strNestCode;
	public static int intGmailMessages;
	public static int intIMAPMessages;
	public static int intRSSUnread;
	public static int intNestStatus; //Home=0,Away=1
	private static Calendar dteNestTokenExpires=null;
	byte[] watchBuffer = new byte[32];
	byte[] outBuffer = new byte[32];
	private static boolean blnSync;
	public static long TimeUpdateInterval = 1800000;
	private static long ThrottleDelay = 30000;
	private static Firebase nestapi=null;
	private static String strNestAccessToken="";

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
	
	private static String NestClientID="9ebcdba4-7e33-40c7-a766-d1e759b75eea";
	private static String NestClientSecret="5KtcNLgrrcu5dTPkGXaujXRZI";

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

			if (tx != null)
			{
				sendTimeUpdate();
				sendNotificationUpdate();
			}

			timerHandler.postDelayed(this, TimeUpdateInterval);
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
		SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMddHHmmss");
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

	@SuppressWarnings("deprecation") private void readSettings()
	{
		SharedPreferences settings = getBaseContext().getSharedPreferences(
				"ArduinoWatch", Context.MODE_PRIVATE);
		strRSSFeedURL = settings.getString("RSSFeedURL", "");
		strRSSFeedUserName = settings.getString("RSSFeedUserName", "");
		strRSSFeedPassword = settings.getString("RSSFeedPassword", "");
		strNestPIN = settings.getString("NestPIN", "");
		strAOLUserName = settings.getString("AOLUserName", "");
		strAOLPassword = settings.getString("AOLPassword", "");
		strWatchBTID = settings.getString("WatchBTID", "");
		strNestAccessToken = settings.getString("NestAccessToken", "");
		dteNestTokenExpires = Calendar.getInstance();
		dteNestTokenExpires.setTimeInMillis(settings.getLong("NestAccessTokenExpires", 0));
	}

	private void initWatchConnection() throws IOException
	{
		adapter = BluetoothAdapter.getDefaultAdapter();
		System.out.println("Service:  Scanning for devices...");
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
					Log.e("error",
							"Service:  Watch Bluetooth Address is invalid.");
				}
			}
			else
			{
				Log.e("error", "Service:  Bluetooth is disabled.");
			}
		}
	}

	public void sendToWatch(String s) throws IOException
	{
		if (tx != null)
		{
			// Update TX characteristic value. Note the setValue overload
			// that
			// takes a byte array must be used.
			tx.setValue(s.getBytes());
			if (gatt.writeCharacteristic(tx))
			{
				System.out.println("Service:  Sent: " + s);
			}
			else
			{
				System.out.println("Service:  Couldn't write TX characteristic!");
			}
		}
	}

	private void sendNotificationUpdate()
	{
		try
		{
			sendToWatch("$nf" + Integer.toString(intGmailMessages) + ","
					+ Integer.toString(intIMAPMessages) + ","
					+ Integer.toString(intRSSUnread) + "," + /*TODO:  Send Nest Status*/ Integer.toString(0));
		}
		catch (IOException e)
		{
			System.out.println("Service:  Connection to watch broken.  Command not sent:"
					+ "$nf"
					+ Integer.toString(intGmailMessages)
					+ ","
					+ Integer.toString(intIMAPMessages)
					+ ","
					+ Integer.toString(intRSSUnread));
		}
	}

	private int checkRSSUnread()
	{	
		if(strRSSFeedURL != "" || strRSSFeedUserName != "" || strRSSFeedPassword != "")
		{
			int intRSSCount = 0;
			JSONObject JOResponse,JOContents;
			String strSessionID="", strRSSStatus="";
	
			// Login to TTRSS
			JOResponse = makeJSONRequest("{\"op\":\"login\",\"user\":\""
					+ strRSSFeedUserName + "\",\"password\":\""
					+ strRSSFeedPassword + "\"}", strRSSFeedURL);
			if (JOResponse != null)
			{
				try
				{
					JOContents = JOResponse.getJSONObject("content");
					if(JOContents != null)
					{
						strSessionID = JOContents.getString("session_id");
						System.out.println("Service:  Logged in to TTRSS.  SessionID:"
								+ strSessionID);
					}
				}
				catch (JSONException e)
				{
					return -1;
				}
			}
			else
			{
				return -1;
			}
			// Get Unread Count from TTRSS
			JOResponse = makeJSONRequest("{\"op\":\"getUnread\",\"sid\":\""+strSessionID+"\"}",
					strRSSFeedURL);
			if (JOResponse != null)
			{
				try
				{
					JOContents = JOResponse.getJSONObject("content");
					if(JOContents != null)
					{
						intRSSCount = JOContents.getInt("unread");
						System.out.println("Service:  Got RSS Count from TTRSS.  Unread:"
								+ intRSSCount);
					}
				}
				catch (JSONException e)
				{
					return -1;
				}
			}
			else
			{
				return -1;
			}
			// Logout from TTRSS
			JOResponse = makeJSONRequest("{\"op\":\"logout\",\"sid\":\""+strSessionID+"\"}", strRSSFeedURL);
			if (JOResponse != null)
			{
				try
				{
					JOContents = JOResponse.getJSONObject("content");
					if(JOContents != null)
					{
						strRSSStatus = JOContents.getString("status");
						System.out.println("Service:  Logged out from TTRSS.  Status:"
								+ strRSSStatus);
					}
				}
				catch (JSONException e)
				{
					// Do nothing
				}
			}
			else
			{
				// Do nothing
			}
	
			return intRSSCount;
		}
		else
		{
			return -1;
		}
	}

	private JSONObject makeJSONRequest(String request, String URL)
	{
		JSONObject retval = null;
		DefaultHttpClient httpclient = new DefaultHttpClient(
				new BasicHttpParams());
		HttpPost httppost = new HttpPost(URL);
		// Depends on your web service
		httppost.setHeader("Content-type", "application/json");
		try
		{
			httppost.setEntity(new StringEntity(request.toString(), "UTF-8"));
		}
		catch (UnsupportedEncodingException e)
		{
			
		}
		
		System.out.println("Service: JSON request:"+request);

		InputStream inputStream = null;
		String result = null;
		try
		{
			HttpResponse response = httpclient.execute(httppost);
			HttpEntity entity = response.getEntity();

			inputStream = entity.getContent();
			// json is UTF-8 by default
			BufferedReader reader = new BufferedReader(
					new InputStreamReader(inputStream, "UTF-8"), 8);
			StringBuilder sb = new StringBuilder();

			String line = null;
			while ((line = reader.readLine()) != null)
			{
				sb.append(line + "\n");
			}
			result = sb.toString();
			System.out.println("Service: JSON result:"+result);
		}
		catch (Exception e)
		{
			// Oops
		}
		finally
		{
			try
			{
				if (inputStream != null)
					inputStream.close();
			}
			catch (Exception squish)
			{
			}
		}
		try
		{
			retval = new JSONObject(result);
		}
		catch (JSONException e)
		{
			System.out.println("Service:  Failed to convert to JSON:  "
					+ result);
		}
		return retval;
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
//		try
//		{
//			if (gatt != null)
//			{
//				if (!gatt.writeCharacteristic(tx))
//				{
//					initWatchConnection();
//				}
//			}
//			else
//			{
//				initWatchConnection();
//			}
//		}
//		catch (IOException e)
//		{
//			System.out.println("Service:  Error Connecting");
//		}
		final Runnable r = new Runnable()
		{
			public void run()
			{
				BluetoothGattCharacteristic oldtx = null;
				int intOldRSS=-1,intOldGmail=-1,intOldIMAP=-1;
				timerHandler.postDelayed(timerRunnable, TimeUpdateInterval);
				while (true)
				{
					getNestStatus();
//					getIMAPCount();
//					getGmailCount();
//					intRSSUnread = checkRSSUnread();
//					if(intOldRSS != intRSSUnread)
//					{
//						System.out.println("Service:  RSS Unread has changed.  Sending Update.");
//						sendNotificationUpdate();
//						intOldRSS=intRSSUnread;
//					}
//					if(intRSSUnread==-1)
//					{
//						intRSSUnread=0;
//					}
//					if (tx != null)
//					{
//						if (tx != oldtx)
//						{
//							// Just connected
//							sendTimeUpdate();
//							sendNotificationUpdate();
//							oldtx = tx;
//						}
//					}
//					else
//					{
//						try
//						{
//							initWatchConnection();
//						}
//						catch (IOException e)
//						{
//							System.out.println("Service:  Failed to Connect to Watch");
//						}
//					}
					// do stuff
					try
					{
						Thread.sleep(ThrottleDelay);
					}
					catch (InterruptedException e)
					{
						// resume loop execution
					}
				}
			}
		};
		Thread t = new Thread(r);
		t.start();
		return Service.START_STICKY;
	}

	protected void getNestStatus()
	{
		JSONObject JOResponse;
		String strBaseURL="https://developer-api.nest.com";
		String strAccessURL= "https://api.home.nest.com/oauth2/access_token?code="+strNestPIN+"&client_id="+NestClientID+"&client_secret="+NestClientSecret+"&grant_type=authorization_code";
		
		if(dteNestTokenExpires == null || dteNestTokenExpires.before(Calendar.getInstance()))
		{
			if(strNestPIN != "")
			{
				System.out.println("Service:  Getting Nest Auth Token");
				JOResponse = makeJSONRequest("", strAccessURL);
				if (JOResponse != null)
				{
					try
					{
						if(!JOResponse.has("error"))
						{
							strNestAccessToken = JOResponse.getString("access_token");
							System.out.println("Service:  Got Nest Access Token:" + strNestAccessToken);
							SharedPreferences settings = getBaseContext().getSharedPreferences("ArduinoWatch", MODE_PRIVATE);
							SharedPreferences.Editor editor = settings.edit();
							editor.putString("NestAccessToken",strNestAccessToken);
							dteNestTokenExpires=Calendar.getInstance();
							dteNestTokenExpires.add(Calendar.SECOND, JOResponse.getInt("expires_in"));
							System.out.println("Service:  Nest Token Expires on "+dteNestTokenExpires);
							editor.putLong("NestAccessTokenExpires", dteNestTokenExpires.getTimeInMillis());
							editor.commit();
						}
					}
					catch (JSONException e)
					{
						return;
					}
				}
				else
				{
					dteNestTokenExpires=null;
					strNestAccessToken="";
					return;
				}
			}
			else
			{
				System.out.println("Service:  No Nest client PIN entered.  Clearing Nest Auth");
				SharedPreferences settings = getBaseContext().getSharedPreferences("ArduinoWatch", MODE_PRIVATE);
				SharedPreferences.Editor editor = settings.edit();
				editor.putString("NestPIN", "");
				editor.putString("NestAccessToken","");
				editor.putLong("NestAccessTokenExpires", 0);
				editor.commit();
			}
		}
		else
		{
			if(nestapi == null)
			{
				System.out.println("Service:  Setting up Nest Monitor");
				nestapi = new Firebase(strBaseURL);
				nestapi.auth(strNestAccessToken, new Firebase.AuthListener() {

					    @Override
					    public void onAuthError(FirebaseError error) {
					        System.out.println("Service:  Nest Login Failed! " + error.getMessage());
					    }

					    @Override
					    public void onAuthSuccess(Object authData) {
					        System.out.println("Service:  Nest Login Succeeded!");
					    }

					    @Override
					    public void onAuthRevoked(FirebaseError error) {
					        System.out.println("Service:  Nest Authentication status was cancelled! " + error.getMessage());
					    }

					}
				);

				nestapi.addValueEventListener(new ValueEventListener() {
					  @Override
					  public void onDataChange(DataSnapshot snapshot) {
					    System.out.println("Service:  Nest output:  " + snapshot.getValue());
					    for(DataSnapshot structures : snapshot.getChildren())
					    {
						    for(DataSnapshot structure : structures.getChildren())
						    {
							    if(structure.hasChild("away") == true)
							    {
								    String strNestStatus = (String) structure.child("away").getValue();
								    System.out.println("Service:  Nest Status = "+strNestStatus);
								    if(strNestStatus.equals("home") == true)
								    {
									    // Nest is Home
									    intNestStatus=0;
								    }
								    else
								    {
									    // Nest is Away or AutoAway
									    intNestStatus=1;
								    }
								    System.out.println("Service:  Set intNestStatus = "+intNestStatus);
							    }
						    }
					    }
					  }

					  @Override
					  public void onCancelled(FirebaseError error) {
					    System.out.println("Service:  Nest Listener was cancelled");
					  }
					}
				);
			}
		}
	}

	@SuppressWarnings(
	{ "unchecked", "rawtypes", "unused" }) private void getGmailCount()
	{
		System.out.println("Service:  Starting getGmailCount()");
		// Get the account list, and pick the first one
		final String ACCOUNT_TYPE_GOOGLE = "com.google";
		final String[] FEATURES_MAIL =
		{ "service_mail" };
		try
		{
			AccountManager.get(this).getAccountsByTypeAndFeatures(
					ACCOUNT_TYPE_GOOGLE, FEATURES_MAIL,
					new AccountManagerCallback()
					{
						@Override public void run(AccountManagerFuture future)
						{
							Account[] accounts = null;
							System.out.println("Service:  In Account Manager callback");
							try
							{
								accounts = (Account[]) future.getResult();
								if (accounts != null && accounts.length > 0)
								{
									String selectedAccount = accounts[0].name;
									System.out.println("Service:  Selected Account "+selectedAccount);
									Cursor labelsCursor = getContentResolver().query(GmailContract.Labels
													.getLabelsUri(selectedAccount),
													null, null, null,
													null);
									System.out.println("Service:  Got Gmail Info");
									// loop through the cursor and find the Inbox
									if (labelsCursor != null) {
									    final String inboxCanonicalName = GmailContract.Labels.LabelCanonicalNames.CANONICAL_NAME_ALL_MAIL;
									    final int canonicalNameIndex = labelsCursor.getColumnIndexOrThrow(GmailContract.Labels.CANONICAL_NAME);
									    while (labelsCursor.moveToNext()) {
									        if (inboxCanonicalName.equals(labelsCursor.getString(canonicalNameIndex))) {
									     	// this row corresponds to the Inbox
									            System.out.println("Service:  Found Gmail Inbox");
									            // Unfortunately, this returns number of unread conversations not messages.
									            intGmailMessages = labelsCursor.getInt(labelsCursor.getColumnIndex("numUnreadConversations"));
									            System.out.println("Service:  Set intGmailMessages to "+intGmailMessages);
									        }
									    }
									}
								}
	
							}
							catch (OperationCanceledException oce)
							{
								// TODO: handle exception
							}
							catch (IOException ioe)
							{
								// TODO: handle exception
							}
							catch (AuthenticatorException ae)
							{
								// TODO: handle exception
							}
						}
					}, null /* handler */);
			// TODO Auto-generated method stub
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	private void getIMAPCount()
	{
		System.out.println("Service:  Starting getIMAPCount()");
		// TODO Auto-generated method stub
		
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
				System.out.println("Service:  Connected!");
				// Discover services.
				if (!gatt.discoverServices())
				{
					System.out.println("Service:  Failed to start discovering services!");
				}
			}
			else if (newState == BluetoothGatt.STATE_DISCONNECTED)
			{
				System.out.println("Service:  Disconnected!");
				tx = null;
				rx = null;
			}
			else
			{
				System.out.println("Service:  Connection state changed. New state: "
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
				System.out.println("Service:  Service discovery completed!");
			}
			else
			{
				System.out.println("Service:  Service discovery failed with status: "
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
				System.out.println("Service:  Couldn't set notifications for RX characteristic!");
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
					System.out.println("Service:  Couldn't write RX client descriptor value!");
				}
			}
			else
			{
				System.out.println("Service:  Couldn't get RX client descriptor!");
			}
		}

		// Called when a remote characteristic changes (like the RX
		// characteristic).
		@Override public void onCharacteristicChanged(BluetoothGatt gatt,
				BluetoothGattCharacteristic characteristic)
		{
			super.onCharacteristicChanged(gatt, characteristic);
			System.out.println("Service:  Received: "
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
			if (bluetoothDevice.getAddress() != null)
			{
				String strFoundDevice = new String(
						bluetoothDevice.getAddress());
				System.out.println("Service:  Found device: "
						+ strFoundDevice);
				if (strFoundDevice.compareTo(strWatchBTID) == 0)
				{
					System.out.println("Service:  Found watch.");
					// Check if the device has the UART service.
					if (parseUUIDs(bytes).contains(UART_UUID))
					{
						// Found a device, stop the scan.
						adapter.stopLeScan(scanCallback);
						System.out.println("Service:  Found UART service!");
						// Connect to the device.
						// Control flow will now go to the callback
						// functions
						// when
						// BTLE events occur.
						gatt = bluetoothDevice.connectGatt(
								getApplicationContext(), false,
								callback);
					}
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
