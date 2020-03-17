package org.elastos.sdk.elephantwallet.contact.internal;

import android.util.Log;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;

import org.elastos.sdk.elephantwallet.contact.Contact;
import org.elastos.sdk.elephantwallet.contact.Utils;
import org.elastos.tools.crosspl.CrossBase;
import org.elastos.tools.crosspl.annotation.CrossClass;
import org.elastos.tools.crosspl.annotation.CrossInterface;

import java.io.File;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

@CrossClass
class ContactBridge extends CrossBase {
    public class SyncInfoLocation {
        public static final int DidChain = 1;
        public static final int Oss = 2;
    }

    public static final String TAG = "elastos";

    protected ContactBridge() {
        super(ContactBridge.class.getName(), 0);
    }

    public void finalize() {
        for (Contact.ChannelStrategy channel : mCustomChannelMap.values()) {
            channel.unbind();
        }
        mCustomChannelMap.clear();

        if (mListener != null) {
            mListener.unbind();
        }
        if (mDataListener != null) {
            mDataListener.unbind();
        }

        super.finalize();
    }

    public synchronized int appendChannelStrategy(Contact.ChannelStrategy channelStrategy) {
        Contact.ChannelStrategy oldChannelStrategy = mCustomChannelMap.get(channelStrategy.getChannelId());
        if (oldChannelStrategy != null) {
            oldChannelStrategy.unbind();
        }

        channelStrategy.bind();
        int ret = appendChannelStrategy((CrossBase) channelStrategy);
        if (ret < 0) {
            channelStrategy.unbind();
            return ret;
        }

        mCustomChannelMap.put(channelStrategy.getChannelId(), channelStrategy);
        return 0;
    }

    public synchronized void setListener(Contact.Listener listener) {
        if (mListener != null) {
            mListener.unbind();
        }
        mListener = listener;

        mListener.bind();
        setListener((CrossBase) mListener);
    }

    public synchronized void setDataListener(Contact.DataListener listener) {
        if (mDataListener != null) {
            mDataListener.unbind();
        }
        mDataListener = listener;

        mDataListener.bind();
        setDataListener((CrossBase) mDataListener);
    }

    public int setIdentifyCode(IdentifyCode.Type type, String value) {
        int ret = setIdentifyCode(type.id(), value);
        return ret;
    }

    public int setUserInfo(HumanInfo.Item item, String value) {
        int ret = setHumanInfo("-user-info-", item.id(), value);
        return ret;
    }

    public Contact.UserInfo getUserInfo() {
        assert (mListener != null);

        Contact.UserInfo userInfo = new Contact.UserInfo();
        int ret = getHumanInfo("-user-info-", userInfo);
        if (ret < 0) {
            Log.w(TAG, "Failed to get user info. ret=" + ret);
            return null;
        }

        return userInfo;
    }

    public int getUserBrief(StringBuffer brief) {
        assert (mListener != null);

        int ret = getHumanBrief("-user-info-", UserInfo.GetCurrDevId(), brief);
        if (ret < 0) {
            Log.w(TAG, "Failed to get user info. ret=" + ret);
            return ret;
        }

        return 0;
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
        List<Contact.FriendInfo.FriendJson> listJson = Utils.GetGsonBuilder().create().fromJson(json.toString(), friendInfoListType.getType());

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

    public int setHumanInfo(String humanCode, HumanInfo.Item item, String value) {
        int ret = setHumanInfo(humanCode, item.id(), value);
        return ret;
    }

    public <T extends HumanInfo> int getHumanInfo(String humanCode, T humanInfo) {
        assert(mListener != null);

        StringBuffer sbInfo = new StringBuffer();
        int ret = getHumanInfo(humanCode, sbInfo);
        if(ret < 0) {
            Log.w(TAG, "Failed to get human info. ret=" + ret);
            return ret;
        }

        ret  = humanInfo.fromJson(sbInfo.toString());
        if(ret < 0) {
            Log.w(TAG, "Failed to deserialize human info. ret=" + ret);
            return ret;
        }

        return 0;
    }

    public String findAvatarFile(String avatar) {
        StringBuffer filepath = new StringBuffer();
        int ret = findAvatarFile(avatar, filepath);
        if(ret < 0) {
            return null;
        }

        return filepath.toString();
    }

    public Contact.Status getStatus(String humanCode) {
        int ret = getHumanStatus(humanCode);
        if(ret < 0) {
            return null;
        }

        return Contact.Status.valueOf(ret);
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

    public int pullFileAsync(String friendCode, ContactChannel channelType,
                        Contact.Message.FileData fileInfo) {
        if(fileInfo == null) {
            return -1;
        }

        int ret = pullData(friendCode, channelType.id(), fileInfo.devId, fileInfo.toString());

        return ret;
    }

    public int cancelPullFile(String friendCode, ContactChannel channelType,
                              Contact.Message.FileData fileInfo) {
        if(fileInfo == null) {
            return -1;
        }

        int ret = cancelPullData(friendCode, channelType.id(), fileInfo.devId, fileInfo.toString());

        return ret;
    }

    public static Contact.Message MakeTextMessage(String text, String cryptoAlgorithm, String memo) {
        Contact.Message msg = new Contact.Message(text, cryptoAlgorithm, memo);
        return msg;
    }

    public static Contact.Message MakeBinaryMessage(byte[] binary, String cryptoAlgorithm, String memo) {
        Contact.Message msg = new Contact.Message(binary, cryptoAlgorithm, memo);
        return msg;
    }

    public static Contact.Message MakeFileMessage(File file, String cryptoAlgorithm, String memo) {
        Contact.Message msg = new Contact.Message(file, cryptoAlgorithm, memo);
        return msg;
    }


//    private static Contact.Message MakeMessage(ContactMessage.Type type, byte[] data, String cryptoAlgorithm) {
//        Contact.Message msg = new Contact.Message(type, data, cryptoAlgorithm);
//        return msg;
//    }

    @CrossInterface
    public static native boolean IsDidFriend(String friendCode);

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
    public native int syncInfoDownload(int fromLocation);

    @CrossInterface
    public native int syncInfoUpload(int toLocation);

    @CrossInterface
    public native int exportUserData(String toFile);

    @CrossInterface
    public native int importUserData(String fromFile);

    @CrossInterface
    public native int setWalletAddress(String name, String value);

    @CrossInterface
    private native int appendChannelStrategy(CrossBase channelStrategy);

    @CrossInterface
    private native void setListener(CrossBase listener);

    @CrossInterface
    private native void setDataListener(CrossBase listener);

    @CrossInterface
    private native int setIdentifyCode(int item, String value);

    @CrossInterface
    private native int setHumanInfo(String humanCode, int item, String value);

    @CrossInterface
    private native int getHumanInfo(String humanCode, StringBuffer info);

    @CrossInterface
    private native int getHumanBrief(String friendCode, String devId, StringBuffer brief);

    @CrossInterface
    public native int findAvatarFile(String avatar, StringBuffer filepath);

    @CrossInterface
    private native int getFriendList(StringBuffer info);

    @CrossInterface
    private native int getHumanStatus(String humanCode);

    @CrossInterface
    private native int sendMessage(String friendCode, int channelType, CrossBase message);

    @CrossInterface
    private native int pullData(String friendCode, int channelType, String devId, String dataId);

    @CrossInterface
    private native int cancelPullData(String friendCode, int channelType, String devId, String dataId);

    private HashMap<Integer, Contact.ChannelStrategy> mCustomChannelMap = new HashMap<>();
    private Contact.Listener mListener;
    private Contact.DataListener mDataListener;

    static { Utils.EnsureNativeLibrary(); }
}
