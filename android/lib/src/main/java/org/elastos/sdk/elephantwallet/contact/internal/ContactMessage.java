package org.elastos.sdk.elephantwallet.contact.internal;

import android.util.Log;

import com.google.gson.Gson;
import com.google.gson.annotations.SerializedName;

import org.elastos.sdk.elephantwallet.contact.Contact;
import org.elastos.tools.crosspl.CrossBase;
import org.elastos.tools.crosspl.annotation.CrossClass;
import org.elastos.tools.crosspl.annotation.CrossInterface;

import java.io.File;

@CrossClass
public class ContactMessage extends CrossBase {
    public enum Type {
        Empty(0x00000000),
        MsgText(0x00000001),
        MsgAudio(0x00000002),
        MsgTransfer(0x00000004),
        MsgImage(0x00000008),
        MsgFile(0x00000010),
        MsgBinary(0x00000020);

        public static Type valueOf(int id) {
            Type[] values = Type.values();
            for(int idx = 0; idx < values.length; idx++) {
                if(values[idx].id == id) {
                    return values[idx];
                }
            }
            return null;
        }

        private Type(int id){
            this.id = id;
        }
        private int id;
    }

    public static class MsgData {
        @Override
        public String toString() {
            String str = new Gson().toJson(this);
            return str;
        }
    }

    public static class TextData extends MsgData {
        public TextData(String text) {
            this.text = text;
        }

        @SerializedName(JsonKey.Text)
        final public String text;
    }

    public static class BinaryData extends MsgData {
        public BinaryData(byte[] binary) {
            this.binary = binary;
        }

        @SerializedName(JsonKey.Binary)
        final public byte[] binary;
    }

    public static class FileData extends MsgData {
        public FileData(File file) {
            devId = UserInfo.GetCurrDevId();
            name = file.getName();
            size = file.length();
            md5 = Utils.getMD5Sum(file);
        }

        // fix json decode and encode different issue
        public static String ConvertId(String id) {
            FileData fileData = new Gson().fromJson(id, FileData.class);
            if (fileData == null) {
                Log.w(Contact.TAG, "FileData.ConvertId() 0 Failed to convert " + id);
            }

            return fileData.toString();
        }

        @SerializedName(JsonKey.DeviceId)
        final public String devId;
        @SerializedName(JsonKey.Name)
        final public String name;
        @SerializedName(JsonKey.Size)
        final public long size;
        @SerializedName(JsonKey.Md5)
        final public String md5;
    }

    public final Type type;
    public MsgData data;
    public final String cryptoAlgorithm;
    public long timestamp;

    public int syncMessageToNative() {
        byte[] msgData = data.toString().getBytes();
        int ret = syncMessageToNative(type.id, msgData, cryptoAlgorithm, timestamp);
        return ret;
    }

    public ContactMessage(String text, String cryptoAlgorithm) {
        this(Type.MsgText, new TextData(text), cryptoAlgorithm);
    }

    public ContactMessage(byte[] binary, String cryptoAlgorithm) {
        this(Type.MsgBinary, new BinaryData(binary), cryptoAlgorithm);
    }


    public ContactMessage(File file, String cryptoAlgorithm) {
        this(Type.MsgFile, new FileData(file), cryptoAlgorithm);
    }

    public ContactMessage(Type type, byte[] data, String cryptoAlgorithm) {
        this(type, (MsgData) null, cryptoAlgorithm);
        if(data != null) {
            this.data = new Gson().fromJson(new String(data), GetDataClass(type));
        }
    }

    private ContactMessage(Type type, MsgData data, String cryptoAlgorithm) {
        super(ContactMessage.class.getName(), 0);

        this.type = type;
        this.data = data;
        this.cryptoAlgorithm = cryptoAlgorithm;
        this.timestamp = System.currentTimeMillis();
    }

    static private Class<? extends MsgData> GetDataClass(Type type) {
        switch (type) {
        case MsgText:
            return TextData.class;
        case MsgBinary:
            return BinaryData.class;
        case MsgFile:
            return FileData.class;
        default:
            return null;
        }
    }

    @CrossInterface
    private native int syncMessageToNative(int type, byte[] data, String cryptoAlgorithm, long timestamp);
} // class Factory
