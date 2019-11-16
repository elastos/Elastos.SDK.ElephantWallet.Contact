//package org.elastos.sdk.elephantwallet.contact.internal;
//
//import android.util.Log;
//
//import org.elastos.sdk.elephantwallet.contact.Contact;
//import org.elastos.tools.crosspl.CrossBase;
//import org.elastos.tools.crosspl.annotation.CrossClass;
//import org.elastos.tools.crosspl.annotation.CrossInterface;
//
//@CrossClass
//public abstract class ContactRemoteFileListener extends CrossBase {
//    public abstract void onReceivedData(String friendCode, ContactChannel channelType,
//                                        Contact.Message.FileData fileInfo, byte[] data);
//    public abstract void onEnd(String friendCode, ContactChannel channelType,
//                               Contact.Message.FileData fileInfo, int result);
//
//    public ContactRemoteFileListener() {
//        super(ContactRemoteFileListener.class.getName(), 0);
//    }
//
//    @CrossInterface
//    private void onReceivedMessage(String humanCode, int channelType,
//                                   String fileName, long fileSize, String fileMd5,
//                                   byte[] data) {
//        Contact.Message.FileData fileInfo = new Contact.Message.FileData();
//
//
//
//        onReceivedMessage(humanCode, channelType, message);
//        return;
//    }
//}
