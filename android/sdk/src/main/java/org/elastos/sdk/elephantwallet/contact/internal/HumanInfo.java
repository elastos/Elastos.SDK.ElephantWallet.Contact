package org.elastos.sdk.elephantwallet.contact.internal;

import com.google.gson.annotations.SerializedName;

import java.util.HashMap;
import java.util.List;

public class HumanInfo {
    public enum Item {
        ChainPubKey(1),
        Did(2),
        ElaAddress(3),
        Nickname(4),
        Avatar(5),
        Gender(6),
        Description(7);

        public static Item valueOf(int id) {
            Item[] values = Item.values();
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

        private Item(int id){
            this.id = id;
        }
        private int id;
    }

    public class HumanJson {
        @SerializedName(JsonKey.HumanInfo)
        HumanInfo humanInfo;
    }

    public class CarrierInfo {
        public class DeviceInfo {
            @SerializedName(JsonKey.DeviceId)
            public String devId;
            @SerializedName(JsonKey.DeviceName)
            public String devName;

            @Override
            public String toString() { return Utils.ToString(this); }
        }

        @SerializedName(JsonKey.CarrierAddr)
        public String usrAddr;
        @SerializedName(JsonKey.CarrierId)
        public String usrId;
        @SerializedName(JsonKey.DeviceInfo)
        public DeviceInfo devInfo;
        @SerializedName(JsonKey.UpdateTime)
        public long updateTime;

        @Override
        public String toString() { return Utils.ToString(this); }
    }

    @SerializedName(JsonKey.BoundCarrierArray)
    public List<CarrierInfo> boundCarrierArray;
    @SerializedName(JsonKey.CommonInfoMap)
    private HashMap<Integer, String> commonInfoMap;
    @SerializedName(JsonKey.WalletAddressMap)
    private HashMap<String, String> walletAddressMap;
    @SerializedName(JsonKey.Status)
    public ContactStatus status;
    @SerializedName(JsonKey.HumanCode)
    public String humanCode;
    @SerializedName(JsonKey.UpdateTime)
    public long updateTime;

    public String chainPubKey;
    public String did;
    public String elaAddress;
    public String nickname;
    public String avatar;
    public String gender;
    public String description;

    protected int fromJsonObj(HumanJson info) {
        this.commonInfoMap = null;
        this.walletAddressMap = info.humanInfo.walletAddressMap;
        this.boundCarrierArray = info.humanInfo.boundCarrierArray;
        this.status = info.humanInfo.status;
        this.humanCode = info.humanInfo.humanCode;
        this.updateTime = info.humanInfo.updateTime;

        this.chainPubKey = info.humanInfo.chainPubKey;
        this.did = info.humanInfo.did;
        this.elaAddress = info.humanInfo.elaAddress;
        this.nickname = info.humanInfo.nickname;
        this.avatar = info.humanInfo.avatar;
        this.gender = info.humanInfo.gender;
        this.description = info.humanInfo.description;

        if(info.humanInfo.commonInfoMap != null) {
            this.chainPubKey = info.humanInfo.commonInfoMap.get(Item.ChainPubKey.id());
            this.did = info.humanInfo.commonInfoMap.get(Item.Did.id());
            this.elaAddress = info.humanInfo.commonInfoMap.get(Item.ElaAddress.id());
            this.nickname = info.humanInfo.commonInfoMap.get(Item.Nickname.id());
            this.avatar = info.humanInfo.commonInfoMap.get(Item.Avatar.id());
            this.gender = info.humanInfo.commonInfoMap.get(Item.Gender.id());
            this.description = info.humanInfo.commonInfoMap.get(Item.Description.id());
        }

        return 0;
    }

    protected int toJsonObj(HumanJson json) {
        json.humanInfo = new HumanInfo();

        json.humanInfo.commonInfoMap = null;
        json.humanInfo.walletAddressMap = this.walletAddressMap;
        json.humanInfo.boundCarrierArray = this.boundCarrierArray;
        json.humanInfo.status = this.status;
        json.humanInfo.humanCode = this.humanCode;
        json.humanInfo.updateTime = this.updateTime;

        json.humanInfo.chainPubKey = this.chainPubKey;
        json.humanInfo.did = this.did;
        json.humanInfo.elaAddress = this.elaAddress;
        json.humanInfo.nickname = this.nickname;
        json.humanInfo.avatar = this.avatar;
        json.humanInfo.gender = this.gender;
        json.humanInfo.description = this.description;

        if(this.commonInfoMap != null) {
            json.humanInfo.chainPubKey = this.commonInfoMap.get(Item.ChainPubKey.id());
            json.humanInfo.did = this.commonInfoMap.get(Item.Did.id());
            json.humanInfo.elaAddress = this.commonInfoMap.get(Item.ElaAddress.id());
            json.humanInfo.nickname = this.commonInfoMap.get(Item.Nickname.id());
            json.humanInfo.avatar = this.commonInfoMap.get(Item.Avatar.id());
            json.humanInfo.gender = this.commonInfoMap.get(Item.Gender.id());
            json.humanInfo.description = this.commonInfoMap.get(Item.Description.id());
        }

        return 0;
    }

    protected HumanInfo() {
    }
} // class Factory
