package org.elastos.sdk.elephantwallet.contact.internal;

public class EventArgs {
    public enum Type {
        StatusChanged(101),
//        ReceivedMessage(102),
//        SentMessage(103),
        FriendRequest(104),
//        FriendStatusChanged(105),
        HumanInfoChanged(106);

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

    public EventArgs(int type, String humanCode, int channelType, byte[] data) {
        this.type = Type.valueOf(type);
        this.humanCode = humanCode;
        this.channelType = ContactChannel.valueOf(channelType);
        this.data = data;
    }

    @Override
    public String toString() {
        return "EventArgs"
                + "[type=" + type
                + ",humanCode=" + humanCode
                + ",channelType=" + channelType
                + ",data=" + data
                +"]";
    }

    public Type type;
    public String humanCode;
    public ContactChannel channelType;
    public byte[] data;
}
