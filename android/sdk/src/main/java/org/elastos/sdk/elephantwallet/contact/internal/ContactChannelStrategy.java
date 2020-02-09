package org.elastos.sdk.elephantwallet.contact.internal;

import org.elastos.sdk.elephantwallet.contact.Contact;
import org.elastos.tools.crosspl.CrossBase;
import org.elastos.tools.crosspl.annotation.CrossClass;

@CrossClass
abstract class ContactChannelStrategy extends CrossBase {
    protected ContactChannelStrategy(String name) {
        super(ContactDataListener.class.getName(), 0);
        mChannel = new Contact.Channel(name);
    }

    public Contact.Channel getChannel() {
        return mChannel;
    }

    public int getChannelId() {
        return mChannel.id();
    }

    Contact.Channel mChannel;
}
