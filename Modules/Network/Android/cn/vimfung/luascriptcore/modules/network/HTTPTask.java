package cn.vimfung.luascriptcore.modules.network;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.os.Handler;
import android.os.Looper;

import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Random;

import cn.vimfung.luascriptcore.LuaEnv;
import cn.vimfung.luascriptcore.LuaExportType;
import cn.vimfung.luascriptcore.LuaFunction;
import cn.vimfung.luascriptcore.LuaValue;
import cn.vimfung.luascriptcore.modules.foundation.Encoding;
import cn.vimfung.luascriptcore.modules.foundation.Path;

/**
 * HTTP任务
 */
public class HTTPTask implements LuaExportType
{
    /**
     * 请求地址
     */
    public String url;

    /**
     * 请求头
     */
    public Map<String, String> headers;

    /**
     * 超时时间，单位秒，默认1分钟，
     */
    public int timeout = 60;

    /**
     * 发起GET请求
     *
     * @param resultHandler 返回回调
     * @param faultHandler  失败回调
     */
    public void get(final LuaFunction resultHandler,
                    final LuaFunction faultHandler)
    {
        if (url == null)
        {
            return;
        }

        //先取消之前请求
        cancel();

        Looper mainLooper = Looper.getMainLooper();
        final Handler mainHandler = new Handler(mainLooper);

        new Thread(new Runnable() {
            @Override
            public void run() {

                try {

                    URL requestURL = new URL(url);
                    _curConn = (HttpURLConnection) requestURL.openConnection();

                    try {

                        _curConn.setRequestMethod("GET");
                        _curConn.setConnectTimeout(timeout * 1000);

                        fillHttpHeaders();

                        _curConn.connect();

                        dealResponse(resultHandler, faultHandler, null);
                    }
                    finally
                    {
                        _curConn.disconnect();
                    }

                }
                catch (final MalformedURLException e)
                {
                    mainHandler.post(new Runnable() {
                        @Override
                        public void run() {

                            if (faultHandler != null)
                            {
                                faultHandler.invoke(new LuaValue[]{new LuaValue(e.getMessage())});
                            }

                        }
                    });

                }
                catch (final IOException e)
                {
                    mainHandler.post(new Runnable() {
                        @Override
                        public void run() {

                            if (faultHandler != null)
                            {
                                faultHandler.invoke(new LuaValue[]{new LuaValue(e.getMessage())});
                            }

                        }
                    });

                }

            }
        }).start();

    }

    /**
     * 发起POST请求
     *
     * @param parameters    请求参数
     * @param resultHandler 返回回调
     * @param faultHandler  失败回调
     */
    public void post(final Map<String, String> parameters,
                     final LuaFunction resultHandler,
                     final LuaFunction faultHandler)
    {
        if (url == null)
        {
            return;
        }

        //取消之前的请求
        cancel();

        Looper mainLooper = Looper.getMainLooper();
        final Handler mainHandler = new Handler(mainLooper);

        new Thread(new Runnable() {

            @Override
            public void run() {

                try
                {

                    URL requestURL = new URL(url);
                    _curConn = (HttpURLConnection) requestURL.openConnection();

                    try
                    {
                        _curConn.setRequestMethod("POST");
                        _curConn.setConnectTimeout(timeout * 1000);

                        fillHttpHeaders();

                        _curConn.setDoOutput(true);
                        _curConn.setDoInput(true);

                        //POST请求不能用缓存，设置为false
                        _curConn.setUseCaches(false);

                        //连接服务器
                        _curConn.connect();

                        //得到httpURLConnection的输出流
                        OutputStream os = _curConn.getOutputStream();
                        try
                        {
                            //向对象输出流写出数据，这些数据将存到内存缓冲区中
                            os.write(getParametersString(parameters).getBytes());
                            //刷新对象输出流，将字节全部写入输出流中
                            os.flush();
                        }
                        finally
                        {
                            //关闭流对象
                            os.close();
                        }

                        dealResponse(resultHandler, faultHandler, null);

                    }
                    finally
                    {
                        _curConn.disconnect();
                    }

                }
                catch (final MalformedURLException e)
                {
                    mainHandler.post(new Runnable() {
                        @Override
                        public void run() {

                            if (faultHandler != null)
                            {
                                faultHandler.invoke(new LuaValue[]{new LuaValue(e.getMessage())});
                            }

                        }
                    });
                }
                catch (final IOException e)
                {
                    mainHandler.post(new Runnable() {
                        @Override
                        public void run() {

                            if (faultHandler != null)
                            {
                                faultHandler.invoke(new LuaValue[]{new LuaValue(e.getMessage())});
                            }

                        }
                    });
                }
            }

        }).start();
    }

    /**
     * 上传文件
     *
     * @param fileParams      文件参数
     * @param parameters      请求参数
     * @param resultHandler   返回回调
     * @param faultHandler    失败回调
     * @param progressHandler 上传进度回调
     */
    public void upload(final HashMap<String, HTTPFile> fileParams,
                       final HashMap<String, String> parameters,
                       final LuaFunction resultHandler,
                       final LuaFunction faultHandler,
                       final LuaFunction progressHandler)
    {
        if (url == null)
        {
            return;
        }

        //取消之前的请求
        cancel();

        Looper mainLooper = Looper.getMainLooper();
        final Handler mainHandler = new Handler(mainLooper);

        new Thread(new Runnable() {

            @Override
            public void run() {

                try
                {
                    //总上传字节
                    long totalUploadedBytes = 0;
                    long uploadedBytes = 0;

                    URL requestURL = new URL(url);
                    _curConn = (HttpURLConnection) requestURL.openConnection();

                    try
                    {
                        _curConn.setRequestMethod("POST");
                        _curConn.setConnectTimeout(timeout * 1000);
                        _curConn.setChunkedStreamingMode(4096);     //分块传输

                        fillHttpHeaders();

                        //生成Boundary String
                        int boundaryStringId = new Random().nextInt(9999999 - 123400) + 123400;
                        String boundaryString = String.format(Locale.getDefault(), "Boundary-%d", boundaryStringId);
                        String contentType = String.format(Locale.getDefault(), "multipart/form-data; boundary=%s", boundaryString);
                        _curConn.addRequestProperty("Content-Type", contentType);

                        _curConn.setDoOutput(true);
                        _curConn.setDoInput(true);

                        //POST请求不能用缓存，设置为false
                        _curConn.setUseCaches(false);

                        //连接服务器
                        _curConn.connect();

                        //得到httpURLConnection的输出流
                        OutputStream os = _curConn.getOutputStream();
                        try
                        {
                            os.write(String.format(Locale.getDefault(), "--%s\r\n", boundaryString).getBytes());
                            String endItemBoundaryString = String.format(Locale.getDefault(), "\r\n--%s\r\n", boundaryString);

                            //填充上传文件数据
                            if (fileParams != null)
                            {
                                //计算总的文件大小
                                totalUploadedBytes = getUploadFilesLength(fileParams);

                                //写入文件数据
                                sendUploadFilesData(fileParams, os, boundaryString, totalUploadedBytes, progressHandler);
                            }

                            //写入请求参数
                            if (parameters != null)
                            {
                                int paramIndex = 0;
                                for (Map.Entry<String, String> entry : parameters.entrySet())
                                {
                                    String contentDisposition = String.format(Locale.getDefault(), "Content-Disposition: form-data; name=\"%s\"\r\n\r\n", Encoding.urlEncode(entry.getKey()));
                                    os.write(contentDisposition.getBytes());
                                    os.write(Encoding.urlEncode(entry.getValue()).getBytes());

                                    if (paramIndex < parameters.size() - 1)
                                    {
                                        os.write(endItemBoundaryString.getBytes());
                                    }

                                    paramIndex ++;
                                }
                            }

                            os.write(String.format(Locale.getDefault(), "\r\n--%s--\r\n", boundaryString).getBytes());

                            //刷新对象输出流，将字节全部写入输出流中
                            os.flush();
                        }
                        finally
                        {
                            //关闭流对象
                            os.close();
                        }

                        dealResponse(resultHandler, faultHandler, null);

                    }
                    finally
                    {
                        _curConn.disconnect();
                    }

                }
                catch (final MalformedURLException e)
                {
                    mainHandler.post(new Runnable() {
                        @Override
                        public void run() {

                            if (faultHandler != null)
                            {
                                faultHandler.invoke(new LuaValue[]{new LuaValue(e.getMessage())});
                            }

                        }
                    });
                }
                catch (final IOException e)
                {
                    mainHandler.post(new Runnable() {
                        @Override
                        public void run() {

                            if (faultHandler != null)
                            {
                                faultHandler.invoke(new LuaValue[]{new LuaValue(e.getMessage())});
                            }

                        }
                    });
                }
            }

        }).start();
    }

    /**
     * 下载文件
     *
     * @param resultHandler   返回回调
     * @param faultHandler    失败回调
     * @param progressHandler 下载进度回调
     */
    public void download(final LuaFunction resultHandler,
                         final LuaFunction faultHandler,
                         final LuaFunction progressHandler)
    {
        if (url == null)
        {
            return;
        }

        //取消之前的请求
        cancel();

        Looper mainLooper = Looper.getMainLooper();
        final Handler mainHandler = new Handler(mainLooper);

        new Thread(new Runnable() {
            @Override
            public void run() {

                try {

                    URL requestURL = new URL(url);
                    _curConn = (HttpURLConnection) requestURL.openConnection();

                    try {

                        _curConn.setRequestMethod("GET");
                        _curConn.setConnectTimeout(timeout * 1000);

                        fillHttpHeaders();

                        _curConn.connect();

                        dealResponse(resultHandler, faultHandler, progressHandler);
                    }
                    finally
                    {
                        _curConn.disconnect();
                    }

                }
                catch (final MalformedURLException e)
                {
                    mainHandler.post(new Runnable() {
                        @Override
                        public void run() {

                            if (faultHandler != null)
                            {
                                faultHandler.invoke(new LuaValue[]{new LuaValue(e.getMessage())});
                            }

                        }
                    });

                }
                catch (final IOException e)
                {
                    mainHandler.post(new Runnable() {
                        @Override
                        public void run() {

                            if (faultHandler != null)
                            {
                                faultHandler.invoke(new LuaValue[]{new LuaValue(e.getMessage())});
                            }

                        }
                    });

                }

            }
        }).start();
    }

    /**
     * 取消请求
     */
    public void cancel()
    {
        if (_curConn != null)
        {
            _curConn.disconnect();
            _curConn = null;
        }
    }

    /**
     * 当前请求
     */
    private HttpURLConnection _curConn;


    /**
     * 填充HTTP请求头
     */
    private void fillHttpHeaders()
    {
        if (headers != null)
        {
            //设置请求头
            for (Map.Entry<String, String> entry : headers.entrySet())
            {
                _curConn.setRequestProperty(entry.getKey(), entry.getValue());
            }
        }
    }

    /**
     * 获取参数字符串
     * @param parameters 请求参数集合
     * @return 字符串
     */
    private String getParametersString(Map<String, String> parameters)
    {
        StringBuilder stringBuffer = new StringBuilder();
        if (parameters != null)
        {
            for (Map.Entry<String, String> entry : parameters.entrySet())
            {
                stringBuffer.append(String.format("%s=%s&", Encoding.urlEncode(entry.getKey()), Encoding.urlEncode(entry.getValue())));
            }

            if (stringBuffer.length() > 0)
            {
                stringBuffer.deleteCharAt(stringBuffer.length() - 1);
            }
        }

        return stringBuffer.toString();
    }

    /**
     * 获取上传文件总长度
     * @param httpFiles 待上传文件集合
     * @return 总上传字节数
     */
    private long getUploadFilesLength(Map<String, HTTPFile> httpFiles)
    {
        long totalUploadedBytes = 0;
        for (Map.Entry<String, HTTPFile> entry : httpFiles.entrySet())
        {
            HTTPFile httpFile = entry.getValue();
            if (!httpFile.path.startsWith("/"))
            {
                AssetFileDescriptor fd = null;
                try
                {
                    fd = LuaEnv.defaultEnv().getAndroidApplicationContext().getAssets().openFd(httpFile.path);
                    totalUploadedBytes += fd.getLength();
                }
                catch (IOException e)
                {
                    //不存在文件
                }

            }
            else if (httpFile.path.startsWith(Path.appPath()))
            {
                String fileName = httpFile.path.substring(Path.appPath().length() + 1);
                AssetFileDescriptor fd = null;
                try
                {
                    fd = LuaEnv.defaultEnv().getAndroidApplicationContext().getAssets().openFd(fileName);
                    totalUploadedBytes += fd.getLength();
                }
                catch (IOException e)
                {
                    //不存在文件
                }
            }
            else
            {
                File file = new File(httpFile.path);
                if (file.isFile())
                {
                    totalUploadedBytes += file.length();
                }
            }
        }

        return totalUploadedBytes;
    }

    /**
     * 发送上传文件数据
     * @param httpFiles 文件集合
     * @param outputStream 输出流
     * @param boundaryString 分隔字符串
     * @param totalUploadedBytes 总上传字节数
     * @param progressHandler 上传进度
     */
    private void sendUploadFilesData(Map<String, HTTPFile> httpFiles,
                                     OutputStream outputStream,
                                     String boundaryString,
                                     long totalUploadedBytes,
                                     final LuaFunction progressHandler) throws IOException
    {
        Looper mainLooper = Looper.getMainLooper();
        final Handler mainHandler = new Handler(mainLooper);
        final String endItemBoundaryString = String.format(Locale.getDefault(), "\r\n--%s\r\n", boundaryString);
        final Context appContext = LuaEnv.defaultEnv().getAndroidApplicationContext();

        long uploadedBytes = 0;
        int fileIndex = 0;
        for (Map.Entry<String, HTTPFile> entry : httpFiles.entrySet())
        {
            HTTPFile httpFile = entry.getValue();

            //判断是否为assets中文件
            if (!httpFile.path.startsWith("/") || httpFile.path.startsWith(Path.appPath()))
            {
                //为assets中文件
                String filePath = httpFile.path;
                if (httpFile.path.startsWith(Path.appPath()))
                {
                    filePath = httpFile.path.substring(Path.appPath().length() + 1);
                }

                InputStream inputStream = null;
                try
                {
                    inputStream = appContext.getAssets().open(filePath);

                    String fileName = filePath.substring(filePath.lastIndexOf("/") + 1);
                    String contentDisposition = String.format(Locale.getDefault(), "Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n", entry.getKey(), fileName);
                    outputStream.write(contentDisposition.getBytes());

                    if (httpFile.mimeType == null)
                    {
                        httpFile.mimeType = "application/octet-stream";
                    }
                    outputStream.write(String.format(Locale.getDefault(), "Content-Type: %s\r\n", httpFile.mimeType).getBytes());

                    if (httpFile.transferEncoding != null)
                    {
                        outputStream.write(String.format(Locale.getDefault(), "Content-Transfer-Encoding: %s\r\n", httpFile.transferEncoding).getBytes());
                    }

                    outputStream.write("\r\n".getBytes());

                    //写入文件数据
                    byte[] buffer = new byte[1024];

                    int hasRead = 0;
                    while ((hasRead = inputStream.read(buffer)) != -1)
                    {
                        outputStream.write(buffer, 0, hasRead);

                        //派发进度
                        uploadedBytes += hasRead;
                        dispatchProgressHandler(mainHandler, progressHandler, totalUploadedBytes, uploadedBytes);
                    }

                    if (fileIndex < httpFiles.size() - 1)
                    {
                        outputStream.write(endItemBoundaryString.getBytes());
                    }

                }
                catch (IOException e)
                {
                    //文件不存在
                }
                finally
                {
                    if (inputStream != null)
                    {
                        try
                        {
                            inputStream.close();
                        }
                        catch (IOException e)
                        {
                            e.printStackTrace();
                        }
                    }
                }

            }
            else
            {
                File file = new File(httpFile.path);
                if (file.exists())
                {
                    String contentDisposition = String.format(Locale.getDefault(), "Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n", entry.getKey(), file.getName());
                    outputStream.write(contentDisposition.getBytes());

                    if (httpFile.mimeType == null)
                    {
                        httpFile.mimeType = "application/octet-stream";
                    }
                    outputStream.write(String.format(Locale.getDefault(), "Content-Type: %s\r\n", httpFile.mimeType).getBytes());

                    if (httpFile.transferEncoding != null)
                    {
                        outputStream.write(String.format(Locale.getDefault(), "Content-Transfer-Encoding: %s\r\n", httpFile.transferEncoding).getBytes());
                    }

                    outputStream.write("\r\n".getBytes());

                    //写入文件数据
                    FileInputStream fileInputStream = new FileInputStream(file);
                    DataInputStream dataInputStream = new DataInputStream(fileInputStream);
                    try
                    {
                        int bytes;
                        byte[] bufferOut = new byte[1024];
                        while ((bytes = dataInputStream.read(bufferOut)) != -1)
                        {
                            outputStream.write(bufferOut, 0, bytes);

                            //派发进度
                            uploadedBytes += bytes;
                            dispatchProgressHandler(mainHandler, progressHandler, totalUploadedBytes, uploadedBytes);
                        }
                    }
                    finally
                    {
                        dataInputStream.close();
                        fileInputStream.close();
                    }

                    if (fileIndex < httpFiles.size() - 1)
                    {
                        outputStream.write(endItemBoundaryString.getBytes());
                    }

                }
            }

            fileIndex ++;
        }

    }

    /**
     * 派发进度事件
     * @param handler 处理队列
     * @param progressHandler   进度事件
     * @param total 总字节数
     * @param current  当前字节数
     */
    private void dispatchProgressHandler(Handler handler, final LuaFunction progressHandler, final long total, final long current)
    {
        if (progressHandler != null)
        {
            handler.post(new Runnable() {
                @Override
                public void run() {
                    progressHandler.invoke(new LuaValue[]{new LuaValue(total), new LuaValue(current)});
                }
            });
        }

    }

    /**
     * 处理回复
     * @param resultHandler 返回回调
     * @param faultHandler 错误回调
     * @param downloadProgress 下载进度
     */
    private void dealResponse(final LuaFunction resultHandler,
                              final LuaFunction faultHandler,
                              final LuaFunction downloadProgress)
    {
        if (_curConn == null)
        {
            return;
        }

        Looper mainLooper = Looper.getMainLooper();
        Handler mainHandler = new Handler(mainLooper);

        InputStream inputStream = null;
        ByteArrayOutputStream output = null;

        try
        {
            int contentLength = _curConn.getContentLength();
            final int statusCode = _curConn.getResponseCode();
            inputStream = _curConn.getInputStream();
            output = new ByteArrayOutputStream();

            //读取数据
            byte[] buffer = new byte[4096];

            long downloadedBytes = 0;
            int n;
            while (-1 != (n = inputStream.read(buffer)))
            {
                output.write(buffer, 0, n);
                downloadedBytes += n;

                //派发进度
                dispatchProgressHandler(mainHandler, downloadProgress, contentLength, downloadedBytes);
            }

            final byte[] responseData = output.toByteArray();

            mainHandler.post(new Runnable() {
                @Override
                public void run() {

                    if (resultHandler != null)
                    {
                        resultHandler.invoke(new LuaValue[]{new LuaValue(statusCode), new LuaValue(responseData)});
                    }

                }
            });

        }
        catch (final IOException e)
        {
            mainHandler.post(new Runnable() {
                @Override
                public void run() {

                    if (faultHandler != null)
                    {
                        faultHandler.invoke(new LuaValue[]{new LuaValue(e.getMessage())});
                    }

                }
            });
        }
        finally
        {
            try
            {
                if (inputStream != null)
                {
                    inputStream.close();
                }

                if (output != null)
                {
                    output.close();
                }
            }
            catch (final IOException e)
            {
                e.printStackTrace();
            }

        }
    }

}
