package org.elastos.sdk.elephantwallet.contact.internal;

import android.util.Log;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;

import org.elastos.sdk.elephantwallet.contact.Contact;
import org.elastos.tools.crosspl.CrossBase;
import org.elastos.tools.crosspl.annotation.CrossClass;
import org.elastos.tools.crosspl.annotation.CrossInterface;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

@CrossClass
public class ContactBridge extends CrossBase {
    public static final String TAG = "elastos";

    protected ContactBridge() {
        super(ContactBridge.class.getName(), 0);
    }

    public void finalize() {
        if(mListener != null) {
            mListener.unbind();
        }

        super.finalize();
    }


    public synchronized void setListener(Contact.Listener listener) {
        if(mListener != null) {
            mListener.unbind();
        }
        mListener = listener;

        mListener.bind();
        setListener(mListener);
    }

    public int setUserInfo(UserInfo.Item item, String value) {
        int ret = setUserInfo(item.id(), value);
        return ret;
    }

    public int setIdentifyCode(IdentifyCode.Type type, String value) {
        int ret = setIdentifyCode(type.id(), value);
        return ret;
    }

    public Contact.UserInfo getUserInfo() {
        assert(mListener != null);

        StringBuffer sbInfo = new StringBuffer();
        int ret = getHumanInfo("-user-info-", sbInfo);
        if(ret < 0) {
            Log.w(TAG, "Failed to get user info. ret=" + ret);
            return null;
        }

        Contact.UserInfo userInfo = new Contact.UserInfo();
        ret  = userInfo.fromJson(sbInfo.toString());
        if(ret < 0) {
            Log.w(TAG, "Failed to deserialize user info. ret=" + ret);
            return null;
        }

        return userInfo;
    }

    public List<Contact.FriendInfo> listFriendInfo() {
        assert(mListener != null);

        StringBuffer json = new StringBuffer();
        int ret = getFriendList(json);
        if(ret < 0) {
            Log.w(TAG, "Failed to list friend info. ret=" + ret);
            return null;
        }

        TypeToken<List<Contact.FriendInfo.FriendJson>> friendInfoListType = new TypeToken<List<Contact.FriendInfo.FriendJson>>(){};
        List<Contact.FriendInfo.FriendJson> listJson = new Gson().fromJson(json.toString(), friendInfoListType.getType());

        List<Contact.FriendInfo> list = new ArrayList<>();
        for(Contact.FriendInfo.FriendJson it: listJson) {
            Contact.FriendInfo friendInfo = new Contact.FriendInfo();
            ret  = friendInfo.fromJson(it);
            if(ret < 0) {
                Log.w(TAG, "Failed to deserialize friend info. ret=" + ret);
                return null;
            }

            list.add(friendInfo);
        }

        return list;
    }

    public List<String> listFriendCode() {
        List<String> friendCodeList = new ArrayList<String>();

        List<Contact.FriendInfo> friendList = listFriendInfo();
        if(friendList == null) {
            return friendCodeList;
        }

        for(Contact.FriendInfo it: friendList) {
            friendCodeList.add(it.humanCode);
        }

        return friendCodeList;
    }

    public ContactStatus getStatus(String humanCode) {
        int ret = getHumanStatus(humanCode);
        if(ret < 0) {
            return null;
        }

        return ContactStatus.valueOf(ret);
    }

    public int sendMessage(String friendCode, ContactChannel channelType, Contact.Message message) {
        if(message == null) {
            return -1;
        }

        int ret = message.syncMessageToNative();
        if(ret < 0) {
            return ret;
        }

        ret = sendMessage(friendCode, channelType.id(), message);
        if(ret < 0) {
            return ret;
        }

        return ret;
    }

    public int pullFile(String friendCode, ContactChannel channelType,
                        Contact.Message.FileData fileInfo) {
        if(fileInfo == null) {
            return -1;
        }

        int ret = pullData(friendCode, channelType.id(), fileInfo.devId, fileInfo.toString());

        return ret;
    }

    public static Contact.Message MakeTextMessage(String text, String cryptoAlgorithm) {
        Contact.Message msg = new Contact.Message(text, cryptoAlgorithm);
        return msg;
    }

    public static Contact.Message MakeFileMessage(File file, String cryptoAlgorithm) {
        Contact.Message msg = new Contact.Message(file, cryptoAlgorithm);
        return msg;
    }

//    private static Contact.Message MakeMessage(ContactMessage.Type type, byte[] data, String cryptoAlgorithm) {
//        Contact.Message msg = new Contact.Message(type, data, cryptoAlgorithm);
//        return msg;
//    }

    @CrossInterface
    public native int start();

    @CrossInterface
    public native int stop();

    @CrossInterface
    public native int addFriend(String friendCode, String summary);

    @CrossInterface
    public native int removeFriend(String friendCode);

    @CrossInterface
    public native int acceptFriend(String friendCode);

    @CrossInterface
    public native int syncInfoDownloadFromDidChain();

    @CrossInterface
    public native int syncInfoUploadToDidChain();

    @CrossInterface
    public native int setWalletAddress(String name, String value);

    @CrossInterface
    private native void setListener(CrossBase listener);

    @CrossInterface
    private native int setUserInfo(int item, String value);

    @CrossInterface
    private native int setIdentifyCode(int item, String value);

    @CrossInterface
    private native int getHumanInfo(String humanCode, StringBuffer info);

    @CrossInterface
    private native int getFriendList(StringBuffer info);

    @CrossInterface
    private native int getHumanStatus(String humanCode);

    @CrossInterface
    private native int sendMessage(String friendCode, int channelType, CrossBase message);

    @CrossInterface
    private native int pullData(String friendCode, int channelType, String devId, String fileInfo);

    private ContactListener mListener;

    static {
        System.loadLibrary("Elastos.SDK.Contact.Jni");
    }
}
