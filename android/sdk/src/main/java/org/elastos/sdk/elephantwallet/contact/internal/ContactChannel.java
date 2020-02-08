package org.elastos.sdk.elephantwallet.contact.internal;

import android.support.annotation.NonNull;

import org.elastos.tools.crosspl.CrossBase;
import org.elastos.tools.crosspl.annotation.CrossClass;

import java.util.ArrayList;

@CrossClass
class ContactChannel extends CrossBase {
    public static <T extends ContactChannel> T valueOf(int id) {
        T[] values = values();
        for(int idx = 0; idx < values.length; idx++) {
            if(values[idx].id() == id) {
                return values[idx];
            }
        }
        return null;
    }

    public static <T extends ContactChannel> T[] values() {
        ContactChannel[] v = mValueList.toArray(new ContactChannel[0]);
        return (T[]) v;
    }

    @Override
    public String toString() {
        return this.name + "(" + this.id + ")";
    }

    public ContactChannel(String name){
        super(ContactChannel.class.getName(), 0);
        ContactChannel lastChannel = mValueList.get(mValueList.size() - 1);
        int id = lastChannel.id + 1;
        init(id, name);
    }

    protected ContactChannel(int id, String name) {
        super(ContactChannel.class.getName(), 0);
        init(id, name);
    }

    int id(){
        return this.id;
    }

    private void init(int id, String name) {
        this.id = id;
        this.name = name;
        mValueList.add(this);
    }

    private int id;
    private String name;
    private static ArrayList<ContactChannel> mValueList = new ArrayList<>();
}
