package com.LeeLab.runmicrofluidicsexperiments;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.provider.MediaStore;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.chaquo.python.PyObject;
import com.chaquo.python.Python;
import com.chaquo.python.android.AndroidPlatform;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Path;
import java.time.*;
import java.util.Date;

public class uploadExp extends AppCompatActivity {

    public static final String expdata = "com.example.runmicrofluidicsexperiments.extra.MESSAGE2";
    public static final String expname = "EXPNAME:";
    public static String x = "";
    public static String y = "";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_upload_exp);

        Intent intent = new Intent();
        intent.setType("*/*");
        intent.setAction(Intent.ACTION_GET_CONTENT);
        startActivityForResult(Intent.createChooser(intent, "Select Sheet"), 2);
    }

    @Override
    protected void onResume(){
        super.onResume();
        //Check if python is launched, if not launch it so we can run pump script
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        //Detects request codes
        if(requestCode==2 && resultCode == Activity.RESULT_OK) {
            Uri selectedSheet = data.getData();
            String mimeType = getContentResolver().getType(selectedSheet);
            Log.e("MIMETYPE", mimeType);
            try {
                Date c = new Date();
                String name = c.toString();
                name = (name.substring(0, 20) + name.substring(24));
                y = name;
                File dir = getDir("experiments", 0);
                File exp = null;
                String path = dir.getAbsolutePath();
                if(mimeType.equals("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet")) {
                    exp = new File(path + "/" + name + ".xlsx"); //"/data/user/0/com.LeeLab.runmicrofluidicsexperiments/app_experiments" +
                }
                else if(mimeType.equals("text/csv")){
                    exp = new File(path + "/" + name + ".csv"); //"/data/user/0/com.LeeLab.runmicrofluidicsexperiments/app_experiments" +
                }
                else{
                    Log.e("FAIL", "File type not supported!");
                    Toast.makeText(getApplicationContext(), "Uploaded file was not .xlsx or .csv", Toast.LENGTH_SHORT).show();
                    finish();
                }
                if(exp.createNewFile()){
                    Log.e("FILE", "Created file " + exp.getName() + " at " + exp.getAbsolutePath());
                }
                else Log.e("FAIL", "Did not create new file");
                //mimetype does not change whether you use box or drive or anything.
                InputStream in =  getContentResolver().openInputStream(selectedSheet); //???
                OutputStream out = new FileOutputStream(exp);
                byte[] buf = new byte[1024];
                int len;
                while((len=in.read(buf))>0){
                    out.write(buf,0,len);
                }
                out.close();
                in.close();
                runPython(exp.getName());
            } catch (IOException e) {
                Toast.makeText(getApplicationContext(), "Some exception occurred", Toast.LENGTH_SHORT).show();
                e.printStackTrace();
            }
        }
        else{
            finish();
        }
    }

    public void runPython(String filename){
        if (!Python.isStarted()) Python.start(new AndroidPlatform(getApplicationContext()));

        Python py = Python.getInstance();
        PyObject pr = py.getModule("pantarhei"); //This object is an instance of the python script
        PyObject res = pr.callAttr("main", filename); //Passing in stringified xlsx or csv file
        x = res.toString();

        TextView tv = findViewById(R.id.expdata_text);
        tv.setText(x);
    }

    public void goBack(View view){
        Intent main = new Intent(this, MainActivity.class);
        main.putExtra("Class", "uploadExp");
        main.putExtra(expdata, x);
        main.putExtra(expname, y);
        startActivity(main);
    }
}
