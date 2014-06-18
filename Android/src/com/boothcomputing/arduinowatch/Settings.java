package com.boothcomputing.arduinowatch;

import java.io.Console;

import android.app.Activity;
import android.app.ActionBar;
import android.app.Fragment;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.ConsoleMessage;
import android.widget.EditText;
import android.os.Build;



public class Settings extends Activity {

	public static String strRSSFeedURL;
	public static String strRSSFeedUserName;
	public static String strRSSFeedPassword;
	public static String strGmailUserName;
	public static String strGmailPassword;
	public static String strAOLUserName;
	public static String strAOLPassword;
	public static String strWatchBTID;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_settings);

		if (savedInstanceState == null) {
			getFragmentManager().beginTransaction()
			.add(R.id.container, new PlaceholderFragment()).commit();
		}
		// Get Settings
		SharedPreferences settings = getBaseContext().getSharedPreferences("ArduinoWatch", MODE_PRIVATE);
		strRSSFeedURL = settings.getString("RSSFeedURL", "");
		strRSSFeedUserName = settings.getString("RSSFeedUserName", "");
		strRSSFeedPassword = settings.getString("RSSFeedPassword", "");
		strGmailUserName = settings.getString("GmailUserName", "");
		strGmailPassword = settings.getString("GmailPassword", "");
		strAOLUserName = settings.getString("AOLUserName", "");
		strAOLPassword = settings.getString("AOLPassword", "");
		strWatchBTID = settings.getString("WatchBTID", "");
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {

		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.settings, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

	/**
	 * A placeholder fragment containing a simple view.
	 */
	public static class PlaceholderFragment extends Fragment {

		public PlaceholderFragment() {
		}

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container,
				Bundle savedInstanceState) {
			View rootView = inflater.inflate(R.layout.fragment_settings,
					container, false);
			
			EditText temp;
			Intent mServiceIntent;
			
			temp = (EditText) rootView.findViewById(R.id.txtRSSFeedURL);
			temp.setText(strRSSFeedURL);
			temp = (EditText) rootView.findViewById(R.id.txtRSSFeedUserName);
			temp.setText(strRSSFeedUserName);
			temp = (EditText) rootView.findViewById(R.id.txtRSSFeedPassword);
			temp.setText(strRSSFeedPassword);
			temp = (EditText) rootView.findViewById(R.id.txtGmailUserName);
			temp.setText(strGmailUserName);
			temp = (EditText) rootView.findViewById(R.id.txtGmailPassword);
			temp.setText(strGmailPassword);
			temp = (EditText) rootView.findViewById(R.id.txtAOLUserName);
			temp.setText(strAOLUserName);
			temp = (EditText) rootView.findViewById(R.id.txtAOLPassword);
			temp.setText(strAOLPassword);
			temp = (EditText) rootView.findViewById(R.id.txtWatchBTID);
			temp.setText(strWatchBTID);
			
			mServiceIntent = new Intent(getActivity(), ArduinoWatchService.class);
			mServiceIntent.setAction("StartSync");
			
			getActivity().startService(mServiceIntent);
			
			return rootView;
		}
	}

	public void onResetClicked(View incomingView)
	{

	}
	
	public void onSaveClicked(View incomingView)
	{
		EditText temp;
		
		System.out.println("Starting Save");
		
		SharedPreferences settings = getBaseContext().getSharedPreferences("ArduinoWatch", MODE_PRIVATE);
		SharedPreferences.Editor editor = settings.edit();
		
		temp = (EditText) findViewById(R.id.txtRSSFeedURL);
		editor.putString("RSSFeedURL",temp.getText().toString());
		temp = (EditText) findViewById(R.id.txtRSSFeedUserName);
		editor.putString("RSSFeedUserName",temp.getText().toString());
		temp = (EditText) findViewById(R.id.txtRSSFeedPassword);
		editor.putString("RSSFeedPassword",temp.getText().toString());
		temp = (EditText) findViewById(R.id.txtGmailUserName);
		editor.putString("GmailUserName",temp.getText().toString());
		temp = (EditText) findViewById(R.id.txtGmailPassword);
		editor.putString("GmailPassword",temp.getText().toString());
		temp = (EditText) findViewById(R.id.txtAOLUserName);
		editor.putString("AOLUserName",temp.getText().toString());
		temp = (EditText) findViewById(R.id.txtAOLPassword);
		editor.putString("AOLPassword",temp.getText().toString());
		temp = (EditText) findViewById(R.id.txtWatchBTID);
		editor.putString("WatchBTID",temp.getText().toString());
		
		editor.commit();
		
		Intent mServiceIntent = new Intent(this, ArduinoWatchService.class);
		mServiceIntent.setAction("ReadSettings");
		
		this.startService(mServiceIntent);
	}
}
