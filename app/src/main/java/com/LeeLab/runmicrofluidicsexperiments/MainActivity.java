package com.LeeLab.runmicrofluidicsexperiments;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    public static final String expdata = "com.example.runmicrofluidicsexperiments.extra.MESSAGE";
    public static String x = ""; //A python processed representation of all pump heights and delays
    public static String howtotext = "";
    public static String currExp = "";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        if(!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
            Toast.makeText(this, R.string.ble_not_supported
                    , Toast.LENGTH_LONG).show();
            finish();
        }

        if(getIntent().getStringExtra("Class") != null) {
            TextView tv;
            switch (getIntent().getStringExtra("Class")) {
                case "howTo":
                    howtotext = getIntent().getStringExtra(howTo.EM);
                    tv = findViewById(R.id.potential_text);
                    tv.setText(howtotext);
                    break;

                case "uploadExp":
                    currExp = getIntent().getStringExtra(uploadExp.expname);
                    tv = findViewById(R.id.potential_text);
                    tv.setText("Current loaded experiment: " + currExp);
                    break;

                default:
                    Log.e("DEFAULT", "Defaulted");
                    break;
            }
        }
    }

    //Uploading an experiment (Sheets or Excel file)
    public void uploadExp(View view){
        Intent uploadExpIntent = new Intent(this, uploadExp.class);
        startActivity(uploadExpIntent);
    }

    //Verify connection to the ESP32 microcontroller, all BLE handled here
    public void bleActivity(View view){
        Intent bleIntent = new Intent(this, mainBLE.class);
        Intent uploadExpIntent = getIntent();
        if(uploadExpIntent.getStringExtra(uploadExp.expdata) != null) { //If there is an experiment to
            x = uploadExpIntent.getStringExtra(uploadExp.expdata);    //get from uploadExp, get it
        }
        if(x.equals("")) x = "noexp"; //Experiment was never uploaded
        bleIntent.putExtra(expdata, x);
        startActivity(bleIntent);
    }

    //Guide activity for users to quickly understand how the app is set up
    public void howToActivity(View view){
        Intent howToIntent = new Intent(this, howTo.class);
        startActivity(howToIntent);
    }
}
