package org.elastos.sdk.elephantwallet.contact.internal;

import java.util.ArrayList;

class ContactStatus<T extends ContactStatus> {
    public static <T extends ContactStatus> T valueOf(int id) {
        T[] values = values();
        for(int idx = 0; idx < values.length; idx++) {
            if(values[idx].id() == id) {
                return values[idx];
            }
        }
        return null;
    }

    public static <T extends ContactStatus> T[] values() {
        ContactStatus[] v = valueList.toArray(new ContactStatus[0]);
        return (T[]) v;
    }

    @Override
    public String toString() {
        return this.name + "(" + this.id + ")";
    }

    int id(){
        return this.id;
    }

    protected ContactStatus(int id, String name){
        this.id = id;
        this.name = name;
        valueList.add((T) this);
    }
    private int id;
    private String name;
    private static ArrayList<ContactStatus> valueList = new ArrayList<>();
}
