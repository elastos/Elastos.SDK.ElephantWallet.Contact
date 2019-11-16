package org.elastos.sdk.elephantwallet.contact.internal;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.annotations.SerializedName;

public class FriendInfo extends HumanInfo {
    public class FriendJson extends HumanJson {
        @SerializedName(JsonKey.Alias)
        String alias;
    }

    public String alias;

    public int fromJson(HumanJson info) {
        if(info instanceof FriendJson == false) {
            return -1;
        }

        int ret = super.fromJsonObj(info);
        if(ret < 0) {
            return ret;
        }

        this.alias = ((FriendJson)info).alias;

        return 0;
    }

    public int fromJson(String info) {
        FriendJson json = new Gson().fromJson(info, FriendJson.class);

        int ret = fromJson(json);
        if(ret < 0) {
            return ret;
        }

        return 0;
    }

    public String toJson() {
        FriendJson json = new FriendJson();

        int ret = super.toJsonObj(json);
        if(ret < 0) {
            return null;
        }

        json.alias = this.alias;

        Gson gson = new GsonBuilder().setPrettyPrinting().create();
        String val = gson.toJson(json);
        return val;
    }
} // class FriendInfo

