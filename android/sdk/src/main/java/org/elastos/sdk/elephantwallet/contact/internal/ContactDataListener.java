package org.elastos.sdk.elephantwallet.contact.internal;

import android.util.Log;

import org.elastos.sdk.elephantwallet.contact.Contact;
import org.elastos.tools.crosspl.CrossBase;
import org.elastos.tools.crosspl.annotation.CrossClass;
import org.elastos.tools.crosspl.annotation.CrossInterface;

import java.nio.ByteBuffer;

@CrossClass
public abstract class ContactDataListener extends CrossBase {
    public enum Status {
        Unknown             (0x0100),
        Initialized         (0x0101),
        Destroyed           (0x0102),
        Connecting          (0x0104),
        Connected           (0x0108),
        Transmitting        (0x0110),
        Closed              (0x0120),
        Failed              (0x0140),

        PeerInitialized     (0x1001),
        PeerFailed          (0x1040);

        public static Status valueOf(int id) {
            Status[] values = Status.values();
            for(int idx = 0; idx < values.length; idx++) {
                if(values[idx].id == id) {
                    return values[idx];
                }
            }
            return null;
        }

        private Status(int id){
            this.id = id;
        }
        private int id;
    }

    public abstract void onNotify(String humanCode, ContactChannel channelType,
                                   String dataId, Status status);
    public abstract int onReadData(String humanCode, ContactChannel channelType,
                                   String dataId, long offset, ByteBuffer data);
    public abstract int onWriteData(String humanCode, ContactChannel channelType,
                                    String dataId, long offset, byte[] data);

    protected ContactDataListener() {
        super(ContactDataListener.class.getName(), 0);
    }

    @CrossInterface
    private void onNotify(String humanCode, int channelType,
                              String dataId, int status) {
        String id = Contact.Message.FileData.ConvertId(dataId);
        if(id == null) {
            id = dataId;
        }

        onNotify(humanCode, ContactChannel.valueOf(channelType),
                id, Status.valueOf(status));
    }

    @CrossInterface
    private byte[] onReadData(String humanCode, int channelType,
                              String dataId, long offset) {
//        Contact.Debug.DumpLocalRefTables();
        String id = Contact.Message.FileData.ConvertId(dataId);
        if(id == null) {
            id = dataId;
        }

        ByteBuffer dataBuf = ByteBuffer.allocate(1024);
        int ret = onReadData(humanCode, ContactChannel.valueOf(channelType),
                             id, offset,
                             dataBuf);
        if(ret < 0) {
            return null;
        }

        int size = ret;
        byte[] data = new byte[size];
        dataBuf.get(data, 0, size);

        return data;
    }

    @CrossInterface
    private int onWriteData(String humanCode, int channelType,
                            String dataId, long offset, byte[] data) {
//        Contact.Debug.DumpLocalRefTables();
        String id = Contact.Message.FileData.ConvertId(dataId);
        if(id == null) {
            id = dataId;
        }

        int ret = onWriteData(humanCode, ContactChannel.valueOf(channelType),
                              id, offset,
                              data);

        return ret;
    }
}
