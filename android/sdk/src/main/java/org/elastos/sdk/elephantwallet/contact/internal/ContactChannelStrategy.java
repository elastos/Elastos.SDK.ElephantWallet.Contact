package org.elastos.sdk.elephantwallet.contact.internal;

import org.elastos.sdk.elephantwallet.contact.Contact;
import org.elastos.tools.crosspl.CrossBase;
import org.elastos.tools.crosspl.annotation.CrossClass;
import org.elastos.tools.crosspl.annotation.CrossInterface;

@CrossClass
abstract class ContactChannelStrategy extends CrossBase {
    protected ContactChannelStrategy(String name) {
        super(ContactChannelStrategy.class.getName(), 0);
        mChannel = new Contact.Channel(name);
    }

    public Contact.Channel getChannel() {
        return mChannel;
    }

    public int getChannelId() {
        return mChannel.id();
    }

    public abstract int onSendMessage(String humanCode, byte[] data);

    public int receivedMessage(String humanCode, byte[] data) {
        int ret = receivedMessage(humanCode, mChannel.id(), data);
        return ret;
    }

    @CrossInterface
    public int onOpen() {
        return 0;
    }

    @CrossInterface
    public int onClose() {
        return 0;
    }

    @CrossInterface
    public int onSendMessage(String humanCode, int channelId, byte[] data) {
        assert(channelId == mChannel.id());

        int ret = onSendMessage(humanCode, data);
        return ret;
    }

    @CrossInterface
    public native int receivedMessage(String humanCode, int channelId, byte[] data);

    Contact.Channel mChannel;
}
