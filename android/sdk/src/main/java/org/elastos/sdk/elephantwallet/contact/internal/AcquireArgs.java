package org.elastos.sdk.elephantwallet.contact.internal;

class AcquireArgs {
    public enum Type {
        PublicKey(201),
        EncryptData(202),
        DecryptData(203),
        DidPropAppId(204),
        DidAgentAuthHeader(205),
        SignData(206);

        public static Type valueOf(int id) {
            Type[] values = Type.values();
            for(int idx = 0; idx < values.length; idx++) {
                if(values[idx].id == id) {
                    return values[idx];
                }
            }
            return null;
        }

        private Type(int id){
            this.id = id;
        }
        private int id;
    }

    public AcquireArgs(int type, String pubKey, byte[] data, String extra) {
        this.type = Type.valueOf(type);
        this.publicKey = pubKey;
        this.data = data;
        this.extra = extra;
    }

    @Override
    public String toString() {
        return "AcquireArgs"
             + "[type=" + type
             + ",publicKey=" + publicKey
             + ",data=" + data
             + ",extra=" + extra
             +"]";
    }

    public Type type;
    public String publicKey;
    public byte[] data;
    public String extra;
}
