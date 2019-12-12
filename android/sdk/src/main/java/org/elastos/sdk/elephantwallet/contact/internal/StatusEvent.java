package org.elastos.sdk.elephantwallet.contact.internal;

import org.elastos.sdk.elephantwallet.contact.Contact;

class StatusEvent extends EventArgs {
    public StatusEvent(int type, String humanCode, int channelType, byte[] data) {
        super(type, humanCode, channelType, data);
        status = Contact.Status.valueOf(data[0]);
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

    public Contact.Status status;
}
