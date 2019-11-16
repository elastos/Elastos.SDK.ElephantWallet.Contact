package org.elastos.sdk.elephantwallet.contact.internal;

public enum ContactChannel {
    Carrier(1),
    ElaChain(2),
    Email(3);

    public static ContactChannel valueOf(int id) {
        ContactChannel[] values = ContactChannel.values();
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

    private ContactChannel(int id){
                       this.id = id;
                                    }
    private int id;
}
