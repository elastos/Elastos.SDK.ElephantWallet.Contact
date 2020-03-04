package org.elastos.sdk.elephantwallet.contact.internal;

import org.elastos.sdk.elephantwallet.contact.Utils;
import org.elastos.tools.crosspl.CrossBase;
import org.elastos.tools.crosspl.annotation.CrossClass;
import org.elastos.tools.crosspl.annotation.CrossInterface;
import org.elastos.tools.crosspl.utils.DataBuffer;

@CrossClass
class KeypairWrapper extends CrossBase {
    protected KeypairWrapper() {
        super(KeypairWrapper.class.getName(), 0);
    }

    @CrossInterface
    public static native int GetSinglePublicKey(byte[] seed, StringBuffer pubKey);

    @CrossInterface
    public static native int GetSinglePrivateKey(byte[] seed, StringBuffer privKey);

    @CrossInterface
    public static native int GenerateMnemonic(String language, String words, StringBuffer mnem);

    @CrossInterface
    public static native int GetSeedFromMnemonic(String mnemonic, String mnemonicPassword, DataBuffer seed);

    @CrossInterface
    public static native int Sign(String privKey, byte[] data, DataBuffer signedData);

    @CrossInterface
    public static native int EciesEncrypt(String pubKey, byte[] plainData, DataBuffer cipherData);

    @CrossInterface
    public static native int EciesDecrypt(String privKey, byte[] cipherData, DataBuffer plainData);

    static { Utils.EnsureNativeLibrary(); }
} // class KeypairWrapper
