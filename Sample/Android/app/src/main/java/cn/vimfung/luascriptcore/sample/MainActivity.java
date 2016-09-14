package cn.vimfung.luascriptcore.sample;

import android.app.Application;
import android.bluetooth.BluetoothClass;
import android.os.Build;
import android.provider.Settings;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;

import cn.vimfung.luascriptcore.LuaContext;
import cn.vimfung.luascriptcore.LuaMethodHandler;
import cn.vimfung.luascriptcore.LuaValue;

public class MainActivity extends AppCompatActivity {

    private LuaContext _luaContext;
    private boolean _hasRegMethod;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        //先拷贝Assets的文件到应用目录中
        File dkjsonFile = new File(getExternalCacheDir(), "dkjson.lua");
        ByteArrayOutputStream dataStream = new ByteArrayOutputStream();
        readAssetFileContent("dkjson.lua", dataStream);
        writeToFile(dkjsonFile, dataStream);

        File todoFile = new File (getExternalCacheDir(), "todo.lua");
        dataStream.reset();
        readAssetFileContent("todo.lua", dataStream);
        writeToFile(todoFile, dataStream);

        File mainFile = new File (getExternalCacheDir(), "main.lua");
        dataStream.reset();
        readAssetFileContent("main.lua", dataStream);
        writeToFile(todoFile, dataStream);

        //创建LuaContext
        _luaContext = LuaContext.create(this);

        //解析脚本按钮点击
        Button evalScriptBtn = (Button) findViewById(R.id.evalScriptButton);
        evalScriptBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                LuaValue retValue = _luaContext.evalScript("print(10);return 'Hello World';");
                Log.v("luaScriptCoreSample", retValue.toString());
            }
        });

        //注册方法按钮点击
        Button regMethodBtn = (Button) findViewById(R.id.regMethodButton);
        regMethodBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                if (!_hasRegMethod)
                {
                    //注册方法
                    _luaContext.registerMethod("getDeviceInfo", new LuaMethodHandler() {
                        @Override
                        public LuaValue onExecute(LuaValue[] arguments) {

                            HashMap devInfoMap = new HashMap();
                            devInfoMap.put("deviceName", new LuaValue(Build.DISPLAY));
                            devInfoMap.put("deviceModel", new LuaValue(Build.MODEL));
                            devInfoMap.put("systemName", new LuaValue(Build.PRODUCT));
                            devInfoMap.put("systemVersion", new LuaValue(Build.VERSION.RELEASE));

                            return new LuaValue(devInfoMap);
                        }
                    });

                    _hasRegMethod = true;
                }

                //调用脚本
                _luaContext.evalScriptFromFile("file:///android_asset/main.lua");
//                File mainFile = new File (getExternalCacheDir(), "main.lua");
//                Log.v("lusScriptCore", mainFile.getAbsolutePath());
//                if (mainFile.exists())
//                {
//                    Log.v("luaScriptCore", "======== has Exists");
//                    _luaContext.evalScriptFromFile(mainFile.getAbsolutePath());
//                }

            }
        });
    }

    /**
     * 读取资源文件内容
     * @param fileName  文件名称
     * @param outputStream  输出内容的二进制流
     */
    private void readAssetFileContent(String fileName, ByteArrayOutputStream outputStream)
    {
        try
        {
            InputStream stream = getAssets().open(fileName);
            byte[] buffer = new byte[1024];

            int hasRead = 0;
            while ((hasRead = stream.read(buffer)) != -1)
            {
                outputStream.write(buffer, 0, hasRead);
            }

            stream.close();
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }

    /**
     * 写入文件
     * @param file  目标文件
     * @param dataStream    二进制流
     */
    private void writeToFile(File file, ByteArrayOutputStream dataStream)
    {
        try
        {
            FileOutputStream stream = new FileOutputStream(file);
            dataStream.writeTo(stream);
            stream.close();
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }
}
