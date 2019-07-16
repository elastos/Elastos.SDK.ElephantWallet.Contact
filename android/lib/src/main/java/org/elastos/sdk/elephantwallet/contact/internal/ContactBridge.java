package org.elastos.sdk.elephantwallet.contact.internal;

import android.util.Log;

import com.google.gson.Gson;

import org.elastos.sdk.elephantwallet.contact.Contact;
import org.elastos.tools.crosspl.CrossBase;
import org.elastos.tools.crosspl.annotation.CrossClass;
import org.elastos.tools.crosspl.annotation.CrossInterface;

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

    public Contact.UserInfo getUserInfo() {
        assert(mListener != null);

        StringBuffer json = new StringBuffer();
        int ret = getHumanInfo("-user-info-", json);
        if(ret < 0) {
            Log.w(TAG, "Failed to get user info. ret=" + ret);
            return null;
        }

        Contact.UserInfo userInfo = new Contact.UserInfo();

        ret  = userInfo.fromJson(json.toString());
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

        List<Contact.FriendInfo> list = new Gson().fromJson(json.toString(), List.class);

        return list;
    }

    public ContactStatus getStatus(String humanCode) {
        int ret = getHumanStatus(humanCode);
        if(ret < 0) {
            return null;
        }

        return ContactStatus.valueOf(ret);
    }

    @CrossInterface
    public native int start();

    @CrossInterface
    public native int addFriend(String friendCode, String summary);

    @CrossInterface
    public native int acceptFriend(String friendCode);

    @CrossInterface
    public native int syncInfoDownloadFromDidChain();

    @CrossInterface
    public native int syncInfoUploadToDidChain();

    @CrossInterface
    private native void setListener(CrossBase listener);

    @CrossInterface
    private native int getHumanInfo(String humanCode, StringBuffer info);

    @CrossInterface
    private native int getFriendList(StringBuffer info);

    @CrossInterface
    private native int getHumanStatus(String humanCode);

    private CrossBase mListener;

    static {
        System.loadLibrary("Elastos.SDK.Contact.Jni");
    }
}
