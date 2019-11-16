package org.elastos.sdk.elephantwallet.contact.internal;

public class StatusEvent extends EventArgs {
    public StatusEvent(int type, String humanCode, int channelType, byte[] data) {
        super(type, humanCode, channelType, data);
        status = ContactStatus.valueOf(data[0]);
    }

    @Override
    public String toString() {
        return "StatusEvent"
                + "[type=" + type
                + ",humanCode=" + humanCode
                + ",channelType=" + channelType
                + ",status=" + status
                +"]";
    }

    public ContactStatus status;
}
