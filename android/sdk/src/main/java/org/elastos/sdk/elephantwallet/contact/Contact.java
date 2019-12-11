package org.elastos.sdk.elephantwallet.contact;

import org.elastos.sdk.elephantwallet.contact.internal.ContactInterface;

public final class Contact extends ContactInterface.Bridge implements ContactInterface {
    public static final class Factory extends ContactInterface.Factory {
        public static Contact Create() {
            return new Contact();
        }

        public static void SetDeviceId(String devId) {
            ContactInterface.Factory.SetDeviceId(devId);
            UserInfo.SetCurrDevId(devId);
        }

        private Factory() { }
    } // class Factory
}
