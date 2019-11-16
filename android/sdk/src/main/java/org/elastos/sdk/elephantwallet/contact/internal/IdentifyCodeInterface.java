package org.elastos.sdk.elephantwallet.contact.internal;

public interface IdentifyCodeInterface {
    public enum Type {
        PhoneNumber(1),
        EmailAddress(2),
        WechatId(3);

        public static Type valueOf(int id) {
            Type[] values = Type.values();
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

        private Type(int id){
            this.id = id;
        }
        private int id;
    }
} // class Factory
