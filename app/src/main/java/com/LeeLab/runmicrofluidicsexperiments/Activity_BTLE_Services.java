package com.LeeLab.runmicrofluidicsexperiments;

import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ExpandableListView;
import android.widget.TextView;
import android.widget.Toast;

import java.security.MessageDigest;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class Activity_BTLE_Services extends AppCompatActivity implements ExpandableListView.OnChildClickListener {
    private final static String TAG = Activity_BTLE_Services.class.getSimpleName();

    public static final String EXTRA_NAME = "com.example.runmicrofluidicsexperiments.Activity_BTLE_Services.NAME";
    public static final String EXTRA_ADDRESS = "com.example.runmicrofluidicsexperiments.Activity_BTLE_Services.ADDRESS";

    private ListAdapter_BTLE_Services expandableListAdapter;
    private ExpandableListView expandableListView;

    private ArrayList<BluetoothGattService> services_ArrayList;
    private HashMap<String, BluetoothGattCharacteristic> characteristics_HashMap;
    private HashMap<String, ArrayList<BluetoothGattCharacteristic>> characteristics_HashMapList;

    private Intent mBTLE_Service_Intent;
    private Service_BTLE_GATT mBTLE_Service;
    private boolean mBTLE_Service_Bound;
    private BroadcastReceiver_BTLE_GATT mGattUpdateReceiver;

    private String name;
    private String address;

    private boolean complete = false;

    private ServiceConnection mBTLE_ServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {

            // We've bound to LocalService, cast the IBinder and get LocalService instance
            Service_BTLE_GATT.BTLeServiceBinder binder = (Service_BTLE_GATT.BTLeServiceBinder) service;
            mBTLE_Service = binder.getService();
            mBTLE_Service_Bound = true;

            if (!mBTLE_Service.initialize()) {
                Log.e(TAG, "Unable to initialize Bluetooth");
                Toast.makeText(getApplicationContext(), "Unable to initialize BLE", Toast.LENGTH_LONG).show();
                finish();
            }

            mBTLE_Service.connect(address);

            // Automatically connects to the device upon successful start-up initialization.
//            mBTLeService.connect(mBTLeDeviceAddress);

//            mBluetoothGatt = mBTLeService.getmBluetoothGatt();
//            mGattUpdateReceiver.setBluetoothGatt(mBluetoothGatt);
//            mGattUpdateReceiver.setBTLeService(mBTLeService);
        }

        @Override
        public void onServiceDisconnected(ComponentName arg0) {
            mBTLE_Service = null;
            mBTLE_Service_Bound = false;

//            mBluetoothGatt = null;
//            mGattUpdateReceiver.setBluetoothGatt(null);
//            mGattUpdateReceiver.setBTLeService(null);
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_btle_services);

        Intent intent = getIntent();
        name = intent.getStringExtra(Activity_BTLE_Services.EXTRA_NAME);
        address = intent.getStringExtra(Activity_BTLE_Services.EXTRA_ADDRESS);

        services_ArrayList = new ArrayList<>();
        characteristics_HashMap = new HashMap<>();
        characteristics_HashMapList = new HashMap<>();

        expandableListAdapter = new ListAdapter_BTLE_Services(
                this, services_ArrayList, characteristics_HashMapList);

        /*
        expandableListView = (ExpandableListView) findViewById(R.id.lv_expandable);
        expandableListView.setAdapter(expandableListAdapter);
        expandableListView.setOnChildClickListener(this);
        */

        ((TextView) findViewById(R.id.tv_name)).setText(name + " Services - Address: " + address);


    }

    @Override
    protected void onStart() {
        super.onStart();


        mGattUpdateReceiver = new BroadcastReceiver_BTLE_GATT(this);
        registerReceiver(mGattUpdateReceiver, Utils.makeGattUpdateIntentFilter());

        mBTLE_Service_Intent = new Intent(this, Service_BTLE_GATT.class);
        bindService(mBTLE_Service_Intent, mBTLE_ServiceConnection, Context.BIND_AUTO_CREATE);
        startService(mBTLE_Service_Intent);
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onStop() {
        super.onStop();

        unregisterReceiver(mGattUpdateReceiver);
        unbindService(mBTLE_ServiceConnection);
        mBTLE_Service_Intent = null;
    }

    //Recover experiment heights and break them down into 515 byte packets or smaller
    public ArrayList<String> parseData(String aggdata){
        ArrayList<String> packs = new ArrayList<String>();
        String packet = "";
        int pointer = 0;
        while(pointer <= aggdata.length()){
            //
            if(pointer + 515 > aggdata.length()) packet = aggdata.substring(pointer);
            else packet = aggdata.substring(pointer, pointer + 515);
            pointer += 515; //Advance pointer by 515 indices in the aggregate data

            packs.add(packet);
        }

        //Testing
        for(int i = 0; i < packs.size(); i++){
            Log.i("PACKET " + i, packs.get(i));
            Log.i("It's size is ", Integer.toString(packs.get(i).length()));
        }
        //End testing
        return packs;
    }

    //Actions when the Go button is tapped
    public void go(View view){
        BluetoothGattCharacteristic characteristic = characteristics_HashMap.get("dee7d621-3fd3-42cb-9498-2042caf3d20f");
        characteristic.setValue("go");
        mBTLE_Service.writeCharacteristic(characteristic);
    }

    //Actions when the pause button is tapped
    public void pause(View view){
        BluetoothGattCharacteristic characteristic = characteristics_HashMap.get("dee7d621-3fd3-42cb-9498-2042caf3d20f");
        characteristic.setValue("pause");
        mBTLE_Service.writeCharacteristic(characteristic);
    }

    //Actions when the reset button is tapped
    public void reset(View view){
        BluetoothGattCharacteristic characteristic = characteristics_HashMap.get("dee7d621-3fd3-42cb-9498-2042caf3d20f");
        characteristic.setValue("reset");
        mBTLE_Service.writeCharacteristic(characteristic);
    }

    //Automatically fire all packets
    public void onClick(View view){
        UpdaterAsyncTask ast = new UpdaterAsyncTask();
        ast.execute();
    }

    public static String SHA256(String value) {
        try{
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(value.getBytes());
            return bytesToHex(md.digest());
        } catch(Exception ex){
            throw new RuntimeException(ex);
        }
    }

    private static String bytesToHex(byte[] bytes) {
        StringBuilder result = new StringBuilder();
        for (byte b : bytes) result.append(Integer.toString((b & 0xff) + 0x100, 16).substring(1));
        return result.toString();
    }

    @Override
    public boolean onChildClick(ExpandableListView parent, View v, int groupPosition, int childPosition, long id) {

        BluetoothGattCharacteristic characteristic = characteristics_HashMapList.get(
                services_ArrayList.get(groupPosition).getUuid().toString())
                .get(childPosition);
        //This function fires only when you click to open the write box...
        Toast.makeText(getApplicationContext(), "???", Toast.LENGTH_LONG).show();

        if (Utils.hasWriteProperty(characteristic.getProperties()) != 0) {
            String uuid = characteristic.getUuid().toString();

            Dialog_BTLE_Characteristic dialog_btle_characteristic = new Dialog_BTLE_Characteristic();

            dialog_btle_characteristic.setTitle(uuid + " (Alan's Custom Characteristic)");
            dialog_btle_characteristic.setService(mBTLE_Service);
            dialog_btle_characteristic.setCharacteristic(characteristic);
            dialog_btle_characteristic.show(getFragmentManager(), "Dialog_BTLE_Characteristic");
        }
        else if (Utils.hasReadProperty(characteristic.getProperties()) != 0) {
            if (mBTLE_Service != null) {
                mBTLE_Service.readCharacteristic(characteristic);
            }
        }
        else if (Utils.hasNotifyProperty(characteristic.getProperties()) != 0) {
            if (mBTLE_Service != null) {
                mBTLE_Service.setCharacteristicNotification(characteristic, true);
            }
        }

        return false;
    }

    //Only maintains hashmaps/arraylists of service and characteristic UUIDs
    public void updateServices() {

        if (mBTLE_Service != null) {

            services_ArrayList.clear();
            characteristics_HashMap.clear();
            characteristics_HashMapList.clear();

            List<BluetoothGattService> servicesList = mBTLE_Service.getSupportedGattServices();

            for (BluetoothGattService service : servicesList) {

                services_ArrayList.add(service);

                List<BluetoothGattCharacteristic> characteristicsList = service.getCharacteristics();
                ArrayList<BluetoothGattCharacteristic> newCharacteristicsList = new ArrayList<>();

                for (BluetoothGattCharacteristic characteristic: characteristicsList) {
                    characteristics_HashMap.put(characteristic.getUuid().toString(), characteristic);
                    newCharacteristicsList.add(characteristic);
                }

                characteristics_HashMapList.put(service.getUuid().toString(), newCharacteristicsList);
            }

            /*
            if (servicesList != null && servicesList.size() > 0) {
                expandableListAdapter.notifyDataSetChanged();
            }
            */
        }
    }

    /* No longer updating characteristics UI wise
    public void updateCharacteristic() {
        expandableListAdapter.notifyDataSetChanged();
    }
    */

    public class UpdaterAsyncTask extends AsyncTask<Void, String, Void> {
        @Override
        protected Void doInBackground(Void... voids) {
            BluetoothGattCharacteristic characteristic = characteristics_HashMap.get("dee7d621-3fd3-42cb-9498-2042caf3d20f");
            int counter = 0;
            String hash = "";
            ArrayList<String> packets = new ArrayList<String>();
            Intent expintent = getIntent();
            if(!expintent.getStringExtra(mainBLE.expdata).equals("noexp")){
                packets = parseData(expintent.getStringExtra(mainBLE.expdata));
                hash = SHA256(expintent.getStringExtra(mainBLE.expdata));
                Log.i("Hash Is ", hash);
            } else {
                //No experiment data was provided
                Toast.makeText(getApplicationContext(), "No experiment data was provided. Cannot continue", Toast.LENGTH_SHORT).show();
                finish();
            }
            if(mBTLE_Service != null) {
                try {
                    //Begin transmission
                    characteristic.setValue("1");
                    mBTLE_Service.writeCharacteristic(characteristic);
                    Thread.sleep(100);
                    hold(characteristic, "confirming"); //Wait until ESP32 changes characteristic to "next" to signal its ready
                    while(counter < packets.size()){ //Bulk of data
                        characteristic.setValue(packets.get(counter));
                        mBTLE_Service.writeCharacteristic(characteristic);
                        Thread.sleep(100);
                        hold(characteristic, ""); //Wait until ESP32 changes characteristic to "next" to signal its ready
                        counter++;
                    }
                    characteristic.setValue("0");
                    mBTLE_Service.writeCharacteristic(characteristic);
                    Thread.sleep(100);
                    hold(characteristic, "hash"); //Wait until ESP32 changes characteristic to "next" to signal its ready
                    characteristic.setValue(hash);
                    mBTLE_Service.writeCharacteristic(characteristic);
                    publishProgress("done");
                    //End transmission
                } catch(InterruptedException ex){
                    Thread.currentThread().interrupt();
                }
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(String... progress) {
            TextView tv = (TextView) findViewById(R.id.tv_progress);
            Log.e("onProgressUpdate", tv.getText().toString());
            Log.e("Progress was", progress[0]);
            if(progress[0].equals("done")) tv.setText("Done. Press Go when ready!");
            else if (progress[0].equals("confirming")) tv.setText("Confirming device is ready to receive data...");
            else if (progress[0].equals("hash")) tv.setText("Sending hash over...");
            else if (tv.getText().toString().equals("Sending packets")) tv.setText("Sending packets.");
            else if (tv.getText().toString().equals("Sending packets.")) tv.setText("Sending packets..");
            else if (tv.getText().toString().equals("Sending packets..")) tv.setText("Sending packets...");
            else if (tv.getText().toString().equals("Sending packets...")) tv.setText("Sending packets....");
            else if (tv.getText().toString().equals("Sending packets....")) tv.setText("Sending packets.....");
            else if (tv.getText().toString().equals("Sending packets.....")) tv.setText("Sending packets");
            else if (progress[0].equals("")) tv.setText("Sending packets");
        }

        //Block until characteristic is set to a value that indicates data has been interpreted
        public void hold(BluetoothGattCharacteristic charac, String progress){
            try{
                while(!charac.getStringValue(0).equals("next")){
                    Log.e(TAG, charac.getStringValue(0));
                    mBTLE_Service.readCharacteristic(charac);
                    Thread.sleep(100);
                    publishProgress(progress);
                }
            } catch(InterruptedException ex) {
                Thread.currentThread().interrupt();
            }
        }
    }
}
