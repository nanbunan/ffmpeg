package com.example.ffmpeg_codec;

import android.os.Bundle;
import android.os.Environment;
import android.app.Activity;
import android.text.TextUtils;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.EditText;

public class MainActivity extends Activity {
	private EditText et_input,et_output;
	private String localPath;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initView();
        Log.i("guan1",Environment.getExternalStorageDirectory().getAbsolutePath());
        Log.i("guan2",Environment.getExternalStorageDirectory().getPath());
    }
private void initView() {
		localPath=Environment.getExternalStorageDirectory().getAbsolutePath()+"/Movies/";
		et_input=(EditText) findViewById(R.id.et_input);
		et_output=(EditText) findViewById(R.id.et_output);
		
	}
public void decode(View view){
	if (et_input.getText().toString().trim()==null|et_output.getText().toString().trim()==null) {
		return;
	}
	Log.i("guan3", localPath+et_input.getText().toString().trim());
	new Thread(new Runnable() {
		
		@Override
		public void run() {
			new VideoCodec().decode(localPath+et_input.getText().toString().trim(), localPath+et_output.getText().toString().trim());
			
		}
	}).start();
	
}

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }
    
}
