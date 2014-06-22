package com.boothcomputing.arduinowatch;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Set;
import java.util.TimeZone;
import java.util.Timer;
import java.util.TimerTask;

import android.app.IntentService;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.ParcelUuid;
import android.preference.PreferenceManager;
import android.util.Log;

public class ArduinoWatchService extends IntentService
{
	public static String strRSSFeedURL;
	public static String strRSSFeedUserName;
	public static String strRSSFeedPassword;
	public static String strGmailUserName;
	public static String strGmailPassword;
	public static String strAOLUserName;
	public static String strAOLPassword;
	public static String strWatchBTID;
	public static String strWatchAuthCode;
	public static int intGmailMessages;
	public static int intIMAPMessages;
	public static int intRSSUnread;
	byte[] watchBuffer=new byte[32];
	byte[] outBuffer=new byte[32];
	private OutputStream outputStream;
	private InputStream inStream;
	private static boolean blnSync;

	public ArduinoWatchService()
	{
		super("ArduinoWatchService");
		
		System.out.println("Service:  Created Service");
		// TODO Auto-generated constructor stub
	}

	@Override protected void onHandleIntent(Intent intent)
	{
		System.out.println("Service:  Processing intent");
		
		switch(intent.getAction())
		{
		case "StartSync":
			System.out.println("Service:  StartSync");
			blnSync=true;
			break;
		case "ReadSettings":
			System.out.println("Service: ReadSettings");
			readSettings();
			break;
		case "StopSync":
			System.out.println("Service:  StopSync");
			blnSync=false;
			break;
		default:
			System.out.println("Service:  Received unknown Intent Action:"+intent.getAction());
			break;
		}
	}

	public void main(String[] args)
	{		
		while(true)
		{
			System.out.println("Service:  Processing main loop");
			readSettings();
			
			try
			{
				initWatchConnection();
			}
			catch (IOException e)
			{
				e.printStackTrace();
			}
			
			try
			{
				readFromWatch();
			}
			catch (IOException e)
			{
				System.out.println("Connection to watch broken.  Unable to read from watch.");
			}
			
			if
			final Handler timerHandler = new Handler();
			Runnable timerRunnable = new Runnable() {

			        @Override
			        public void run() {
			     	   sendTimeUpdate();
			     	   sendNotificationUpdate();

			            timerHandler.postDelayed(this, 500);
			        }
			    };
			
		}

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
		SharedPreferences settings = getBaseContext().getSharedPreferences("ArduinoWatch", Context.MODE_PRIVATE);
		strRSSFeedURL = settings.getString("RSSFeedURL", "");
		strRSSFeedUserName = settings.getString("RSSFeedUserName", "");
		strRSSFeedPassword = settings.getString("RSSFeedPassword", "");
		strGmailUserName = settings.getString("GmailUserName", "");
		strGmailPassword = settings.getString("GmailPassword", "");
		strAOLUserName = settings.getString("AOLUserName", "");
		strAOLPassword = settings.getString("AOLPassword", "");
		strWatchBTID = settings.getString("WatchBTID", "");
		strWatchAuthCode = settings.getString("WatchAuthCode", "");
	}

	private void initWatchConnection() throws IOException
	{
		BluetoothAdapter blueAdapter = BluetoothAdapter.getDefaultAdapter();
		if (blueAdapter != null)
		{
			if (blueAdapter.isEnabled())
			{
				if(BluetoothAdapter.checkBluetoothAddress(strWatchBTID) == true)
				{
					BluetoothDevice bdWatch = blueAdapter.getRemoteDevice(strWatchBTID);
					ParcelUuid[] uuids = bdWatch.getUuids();
					BluetoothSocket socket = bdWatch.createRfcommSocketToServiceRecord(uuids[0].getUuid());
					socket.connect();
					outputStream = socket.getOutputStream();
					inStream = socket.getInputStream();
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
		try
		{
			outputStream.write(s.getBytes());
		}
		catch (IOException e)
		{
			throw e;
		}
	}
	
	public void readFromWatch() throws IOException
	{
		int bytesRead;
		
		try
		{
			// Try reading from watch buffer
			bytesRead=inStream.read(watchBuffer, 0, 32);
			
			//check to see if we got data.  If so, process it.
			if(bytesRead != -1)
			{
				processWatchString(new String(watchBuffer,0,bytesRead));
			}
		}
		catch (IOException e)
		{
			throw e;
		}
	}

	public void processWatchString(String receivedString)
	{
		String[] commands;
		
		commands=receivedString.split("$");
		
		for(String command : commands)
		{
			if(command.startsWith("$tm"))
			{
				//Send time update
				sendTimeUpdate();
			}
			if(command.startsWith("$nf"))
			{
				//Send Notification update
				sendNotificationUpdate();
			}
			if(command.startsWith("$ah"))
			{
				//Send Auth Code
				sendAuthCode();
			}
		}
	}

	private void sendNotificationUpdate()
	{
		try
		{
			sendToWatch("$nf"+Integer.toString(intGmailMessages)+","+Integer.toString(intIMAPMessages)+","+Integer.toString(intRSSUnread));
		}
		catch (IOException e)
		{
			System.out.println("Connection to watch broken.  Command not sent:"
					+ "$nf"+Integer.toString(intGmailMessages)+","+Integer.toString(intIMAPMessages)+","+Integer.toString(intRSSUnread));
		}
	}

	private void sendAuthCode()
	{
		try
		{
			sendToWatch("$ah"+strWatchAuthCode);
		}
		catch (IOException e)
		{
			System.out.println("Connection to watch broken.  Command not sent:"
					+ "$ah"+strWatchAuthCode);
		}
	}

}
