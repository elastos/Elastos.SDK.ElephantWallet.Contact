package org.elastos.tools.crosspl.processor

import javax.lang.model.element.TypeElement
import javax.lang.model.type.ArrayType
import javax.lang.model.type.DeclaredType
import javax.lang.model.type.TypeKind
import javax.lang.model.type.TypeMirror

enum class CrossVariableType {
    BOOLEAN,
    INTEGER,
    LONG,
    DOUBLE,
    VOID,

    STRING,
    BYTEARRAY,
    FUNCTION,

    STRINGBUFFER,
    BYTEBUFFER,

    CROSSBASE;

    companion object {
        fun Parse(type: TypeMirror): CrossVariableType? {
            if(supportedTypeKind.contains(type.kind)) {
                return supportedTypeKind.getValue(type.kind)
            } else if(type is DeclaredType) {
                val typeElement = type.asElement() as TypeElement
                val typeStr = typeElement.toString()
                return supportedTypeDeclared[typeStr]
            } else if(type is ArrayType) {
                val typeStr = type.toString()
                return supportedTypeDeclared[typeStr]
            }

            return null
        }

        private val supportedTypeKind = mapOf(
            TypeKind.BOOLEAN           to BOOLEAN,
            TypeKind.INT               to INTEGER,
            TypeKind.LONG              to LONG,
            TypeKind.DOUBLE            to DOUBLE,
            TypeKind.VOID              to VOID
        )
        private val supportedTypeDeclared = mapOf(
            "java.lang.String"                              to STRING,
            "byte[]"                                        to BYTEARRAY,
            "java.lang.Runnable"                            to FUNCTION,
            "java.lang.StringBuffer"                        to STRINGBUFFER,
            "org.elastos.tools.crosspl.utils.DataBuffer"    to BYTEBUFFER,
            "org.elastos.tools.crosspl.CrossBase"           to CROSSBASE
        )
    }

    fun isPrimitiveType(): Boolean {
        val primitiveTypeSet = setOf(
            BOOLEAN,
            INTEGER,
            LONG,
            DOUBLE,
            VOID
        )

        return primitiveTypeSet.contains(this)
    }

    fun toCppString(isConst: Boolean = false): String {
        val primitiveTypeMap = mapOf(
            BOOLEAN    to "bool",
            INTEGER    to "int",
            LONG       to "int64_t",
            DOUBLE     to "double",
            VOID       to "void"
        )
        val classTypeMap = mapOf(
            STRING       to "const char*",
            BYTEARRAY    to "std::span<uint8_t>",
            FUNCTION     to "std::function<void()>",
            STRINGBUFFER to "std::stringstream",
            BYTEBUFFER   to "std::vector<uint8_t>",
            CROSSBASE    to "crosspl::native::CrossBase"
        )

        var cppType = toString(primitiveTypeMap, classTypeMap, isConst)

        return cppType
    }

    fun toJniString(isConst: Boolean = false): String {
        val primitiveTypeMap = mapOf(
            BOOLEAN    to "jboolean",
            INTEGER    to "jint",
            LONG       to "jlong",
            DOUBLE     to "jdouble",
            VOID       to "void"
        )
        val classTypeMap = mapOf(
            STRING       to "jstring",
            BYTEARRAY    to "jbyteArray",
            FUNCTION     to "jobject",
            STRINGBUFFER to "jobject",
            BYTEBUFFER   to "jobject",
            CROSSBASE    to "jobject"
        )

        var jniType = toString(primitiveTypeMap, classTypeMap, isConst)

        return jniType
    }

    fun toJniSigChar(): String {
        val primitiveTypeMap = mapOf(
            BOOLEAN    to "Z",
            INTEGER    to "I",
            LONG       to "J",
            DOUBLE     to "D",
            VOID       to "V"
        )
        val classTypeMap = mapOf(
            STRING       to "Ljava/lang/String;",
            BYTEARRAY    to "[B",
            FUNCTION     to "Ljava/lang/Runnable;",
            STRINGBUFFER to "Ljava/lang/StringBuffer;",
            BYTEBUFFER   to "Lorg/elastos/tools/crosspl/utils/DataBuffer;",
            CROSSBASE    to "Lorg/elastos/tools/crosspl/CrossBase;"
        )

        var javaChar = toString(primitiveTypeMap, classTypeMap)

        return javaChar
    }

    fun toString(primitiveTypeMap: Map<CrossVariableType, String>,
                 classTypeMap: Map<CrossVariableType, String>,
                 isConst: Boolean = false): String {
        var cppType = primitiveTypeMap[this]
        if(cppType == null) {
            cppType = classTypeMap[this]
        }

        if(isConst
        && ! primitiveTypeMap.contains(this)
        && this != STRING) {
            cppType = "const ${cppType!!}*"
        }

        return cppType!!
    }

    override
    fun toString(): String {
        val stringMap = mapOf(
            BOOLEAN      to "Boolean",
            INTEGER      to "Int",
            LONG         to "Long",
            DOUBLE       to "Double",
            VOID         to "Void",
            STRING       to "String",
            BYTEARRAY    to "ByteArray",
            FUNCTION     to "Function",
            STRINGBUFFER to "StringBuffer",
            BYTEBUFFER   to "ByteBuffer",
            CROSSBASE    to "CrossBase"
        )

        return stringMap.getValue(this)
    }
}
