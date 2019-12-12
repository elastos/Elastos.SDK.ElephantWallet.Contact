package org.elastos.sdk.elephantwallet.contact.internal;

import android.util.Log;

import org.elastos.sdk.elephantwallet.contact.Contact;
import org.elastos.sdk.elephantwallet.contact.Utils;
import org.elastos.tools.crosspl.CrossBase;
import org.elastos.tools.crosspl.annotation.CrossClass;
import org.elastos.tools.crosspl.annotation.CrossInterface;

@CrossClass
abstract class ContactListener extends CrossBase {
    @CrossInterface
    public abstract void onError(int errCode, String errStr, String ext);

    public abstract byte[] onAcquire(AcquireArgs request);
    public abstract void onEvent(EventArgs event);
    public abstract void onReceivedMessage(String humanCode, Contact.Channel channelType, Contact.Message message);

    public class AcquireArgs extends org.elastos.sdk.elephantwallet.contact.internal.AcquireArgs {
        private AcquireArgs(int type, String pubKey, byte[] data) {
            super(type, pubKey, data);
        }
    }

    public class EventArgs extends org.elastos.sdk.elephantwallet.contact.internal.EventArgs {
        public EventArgs(int type, String humanCode, int channelType, byte[] data) {
            super(type, humanCode, channelType, data);
        }
    }

    public class StatusEvent extends EventArgs {
        public StatusEvent(int type, String humanCode, int channelType, byte[] data) {
            super(type, humanCode, channelType, data);
            status = Contact.Status.valueOf(data[0]);
        }
        @Override
        public String toString() {
            return "StatusEvent" + "[type=" + type
                    + ",humanCode=" + humanCode + ",channelType=" + channelType
                    + ",status=" + status +"]";
        }

        public Contact.Status status;
    }

    public class RequestEvent extends EventArgs {
        public RequestEvent(int type, String humanCode, int channelType, byte[] data) {
            super(type, humanCode, channelType, data);
            summary = new String(data);
        }
        @Override
        public String toString() {
            return "RequestEvent" + "[type=" + type
                    + ",humanCode=" + humanCode + ",channelType=" + channelType
                    + ",summary=" + summary +"]";
        }

        public String summary;
    }

    public class InfoEvent extends EventArgs {
        public InfoEvent(int type, String humanCode, int channelType, byte[] data) {
            super(type, humanCode, channelType, data);

            String info = new String(data);
            if(info.contains(JsonKey.IsMyself) == true) {
                UserInfo userInfo = new UserInfo();
                userInfo.fromJson(info);
                humanInfo = userInfo;
            } else if(info.contains(JsonKey.IsFriend) == true) {
                FriendInfo friendInfo = new FriendInfo();
                friendInfo.fromJson(info);
                humanInfo = friendInfo;
            } else {
               Log.w(Utils.TAG, "InfoEvent: Failed to parse human data.");
            }
        }
        @Override
        public String toString() {
            return "InfoEvent" + "[type=" + type
                    + ",humanCode=" + humanCode + ",channelType=" + channelType
                    + ",humanInfo=" + humanInfo +"]";
        }

        public HumanInfo humanInfo;
    }

    protected ContactListener() {
        super(ContactListener.class.getName(), 0);
    }

    @CrossInterface
    private byte[] onAcquire(int reqType, String pubKey, byte[] data) {
        Log.i(Utils.TAG, "ContactListener.onAcquire()");

        AcquireArgs args = new AcquireArgs(reqType, pubKey, data);
        byte[] ret = onAcquire(args);

        return ret;
    }

    @CrossInterface
    private void onEvent(int eventType, String humanCode, int channelType, byte[] data) {

        EventArgs args = null;

        EventArgs.Type type = EventArgs.Type.valueOf(eventType);
        switch (type) {
            case StatusChanged:
                args = new StatusEvent(eventType, humanCode, channelType, data);
                break;
            case FriendRequest:
                args = new RequestEvent(eventType, humanCode, channelType, data);
                break;
            case HumanInfoChanged:
                args = new InfoEvent(eventType, humanCode, channelType, data);
                break;
            default:
                throw new RuntimeException("Unimplemented type: " + type);
        }

        Log.i(Utils.TAG, "ContactListener.onEvent() args=" + args);
        onEvent(args);
        return;
    }

    @CrossInterface
    private void onReceivedMessage(String humanCode, int channelType,
                                   int type, byte[] data, String cryptoAlgorithm, long timestamp) {
        Contact.Message message = new Contact.Message(ContactMessage.Type.valueOf(type),
                                                      data, cryptoAlgorithm);
        message.timestamp = timestamp;

        onReceivedMessage(humanCode, ContactChannel.valueOf(channelType), message);
        return;
    }
}
