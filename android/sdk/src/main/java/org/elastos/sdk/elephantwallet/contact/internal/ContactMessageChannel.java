package org.elastos.sdk.elephantwallet.contact.internal;

import android.util.Log;

import com.google.gson.Gson;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

import org.elastos.sdk.elephantwallet.contact.Contact;
import org.elastos.sdk.elephantwallet.contact.Utils;
import org.elastos.tools.crosspl.CrossBase;
import org.elastos.tools.crosspl.annotation.CrossClass;
import org.elastos.tools.crosspl.annotation.CrossInterface;
import org.jetbrains.annotations.NotNull;

import java.io.File;
import java.util.Random;

//@CrossClass
class ContactMessageChannel extends CrossBase {
    protected ContactMessageChannel(String name) {
        super(ContactMessageChannel.class.getName(), 0);

        mChannel = new Contact.Channel(name);
    }

    Contact.Channel getChannel(){
        return mChannel;
    }

    private Contact.Channel mChannel;
} // class Factory
