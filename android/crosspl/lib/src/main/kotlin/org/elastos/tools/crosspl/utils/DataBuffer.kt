package org.elastos.tools.crosspl.utils

import java.io.ByteArrayOutputStream

open class DataBuffer()
{
    open fun append(b: ByteArray?) {
        buffer.write(b)
    }

    open fun clear() {
        buffer.reset()
    }

    override fun toString(): String {
        return buffer.toString();
    }

    open fun toBytes(): ByteArray {
        return buffer.toByteArray();
    }

    var buffer = ByteArrayOutputStream()
}
