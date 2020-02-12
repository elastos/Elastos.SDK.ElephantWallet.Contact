package org.elastos.sdk.elephantwallet.contact.internal;

import android.support.annotation.NonNull;

import java.util.ArrayList;

class ContactChannel {
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
        ContactChannel[] v = sValueList.toArray(new ContactChannel[0]);
        return (T[]) v;
    }

    @Override
    public String toString() {
        return this.name + "(" + this.id + ")";
    }

    int id(){
        return this.id;
    }

    protected ContactChannel(int id, String name){
        this.id = id;
        this.name = name;
        sValueList.add(this);
    }

    private int id;
    private String name;
    private static ArrayList<ContactChannel> sValueList = new ArrayList<>();
}
