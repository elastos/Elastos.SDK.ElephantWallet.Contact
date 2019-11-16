package org.elastos.sdk.elephantwallet.contact.internal;

public enum ContactStatus {
    Invalid(0x0),
    WaitForAccept(0x1),
    Offline(0x2),
    Online(0x4),
    Removed(0x8);

    public static ContactStatus valueOf(int id) {
        ContactStatus[] values = ContactStatus.values();
        for(int idx = 0; idx < values.length; idx++) {
            if(values[idx].id == id) {
                return values[idx];
            }
        }

        return null;
    }

    int id(){
        return this.id;
    }

    private ContactStatus(int id){
                       this.id = id;
                                    }
    private int id;
}
