package com.LeeLab.runmicrofluidicsexperiments;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;

public class howTo extends AppCompatActivity {

    /* Practicing with RecyclerView ~~~
    private RecyclerView rv;
    private RecyclerView.Adapter mAdapter;
    private RecyclerView.LayoutManager lm;
    private String[] emptySet;
    */

    public static final int HOWTO_SENT = 3;
    public static String EM = "com.example.runmicrofluidicsexperiments.extra.MESSAGE";
    public static String msgback = "";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_how_to);


        /* Practicing with RecyclerView ~~~
        rv = findViewById(R.id.howto_rv);
        rv.setHasFixedSize(true);
        //Use a linear layout manager
        lm = new LinearLayoutManager(this);
        rv.setLayoutManager(lm);

        //Specify your adapter (so you can feed data to the list otherwise it is empty)
        mAdapter = new AlanAdapter(emptySet);
        rv.setAdapter(mAdapter);

        */
        //towst("test");
    }

    public void onClick(View view){
        switch(view.getId()) {
            case R.id.galaxy:
                //handle step1 image tap
                msgback = "Tap on upload experiment! (Supported files: xlsx, xls, csv)";
                break;
            case R.id.galaxy2:
                // handle step2 image tap
                msgback = "Tap on \"Connect to ESP32\"!";
                break;
            default:
                throw new RuntimeException("Unknown button ID");
        }
        Intent intent = new Intent(this, MainActivity.class);
        intent.putExtra(EM, msgback);
        intent.putExtra("Class", "howTo");
        startActivity(intent);
    }
}
