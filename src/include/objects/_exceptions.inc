/*
 Copyright (c) 2012-2013, The Saffire Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the <organization> nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

void object_exception_add_generated_exceptions(void) {
    //SystemException
    Object_Exception_SystemException_struct.attributes = Object_Exception_struct.attributes;
    vm_populate_builtins("systemException", Object_SystemException);
    
    //IndexException
    Object_Exception_IndexException_struct.attributes = Object_Exception_struct.attributes;
    vm_populate_builtins("indexException", Object_IndexException);
    
    //AttributeException
    Object_Exception_AttributeException_struct.attributes = Object_Exception_struct.attributes;
    vm_populate_builtins("attributeException", Object_AttributeException);
    
    //TypeException
    Object_Exception_TypeException_struct.attributes = Object_Exception_struct.attributes;
    vm_populate_builtins("typeException", Object_TypeException);
    
    //CallException
    Object_Exception_CallException_struct.attributes = Object_Exception_struct.attributes;
    vm_populate_builtins("callException", Object_CallException);
    
    //ArgumentException
    Object_Exception_ArgumentException_struct.attributes = Object_Exception_struct.attributes;
    vm_populate_builtins("argumentException", Object_ArgumentException);
    
    //CallableException
    Object_Exception_CallableException_struct.attributes = Object_Exception_struct.attributes;
    vm_populate_builtins("callableException", Object_CallableException);
    
    //VisibilityException
    Object_Exception_VisibilityException_struct.attributes = Object_Exception_struct.attributes;
    vm_populate_builtins("visibilityException", Object_VisibilityException);
    
    //ImportException
    Object_Exception_ImportException_struct.attributes = Object_Exception_struct.attributes;
    vm_populate_builtins("importException", Object_ImportException);
    
    //ArithmeticException
    Object_Exception_ArithmeticException_struct.attributes = Object_Exception_struct.attributes;
    vm_populate_builtins("arithmeticException", Object_ArithmeticException);
    
    //DivideByZeroException
    Object_Exception_DivideByZeroException_struct.attributes = Object_Exception_struct.attributes;
    vm_populate_builtins("divideByZeroException", Object_DivideByZeroException);
    
    //IoException
    Object_Exception_IoException_struct.attributes = Object_Exception_struct.attributes;
    vm_populate_builtins("ioException", Object_IoException);
    
    //FileException
    Object_Exception_FileException_struct.attributes = Object_Exception_struct.attributes;
    vm_populate_builtins("fileException", Object_FileException);
    
    //FileNotFoundException
    Object_Exception_FileNotFoundException_struct.attributes = Object_Exception_struct.attributes;
    vm_populate_builtins("fileNotFoundException", Object_FileNotFoundException);
    
}


t_exception_object Object_Exception_SystemException_struct = { OBJECT_HEAD_INIT_WITH_BASECLASS("systemException", objectTypeException, OBJECT_TYPE_INSTANCE | OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE, &exception_funcs, &Object_Exception_struct), "", 0};
t_exception_object Object_Exception_IndexException_struct = { OBJECT_HEAD_INIT_WITH_BASECLASS("indexException", objectTypeException, OBJECT_TYPE_INSTANCE | OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE, &exception_funcs, &Object_Exception_SystemException_struct), "", 0};
t_exception_object Object_Exception_AttributeException_struct = { OBJECT_HEAD_INIT_WITH_BASECLASS("attributeException", objectTypeException, OBJECT_TYPE_INSTANCE | OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE, &exception_funcs, &Object_Exception_SystemException_struct), "", 0};
t_exception_object Object_Exception_TypeException_struct = { OBJECT_HEAD_INIT_WITH_BASECLASS("typeException", objectTypeException, OBJECT_TYPE_INSTANCE | OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE, &exception_funcs, &Object_Exception_SystemException_struct), "", 0};
t_exception_object Object_Exception_CallException_struct = { OBJECT_HEAD_INIT_WITH_BASECLASS("callException", objectTypeException, OBJECT_TYPE_INSTANCE | OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE, &exception_funcs, &Object_Exception_SystemException_struct), "", 0};
t_exception_object Object_Exception_ArgumentException_struct = { OBJECT_HEAD_INIT_WITH_BASECLASS("argumentException", objectTypeException, OBJECT_TYPE_INSTANCE | OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE, &exception_funcs, &Object_Exception_CallException_struct), "", 0};
t_exception_object Object_Exception_CallableException_struct = { OBJECT_HEAD_INIT_WITH_BASECLASS("callableException", objectTypeException, OBJECT_TYPE_INSTANCE | OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE, &exception_funcs, &Object_Exception_CallException_struct), "", 0};
t_exception_object Object_Exception_VisibilityException_struct = { OBJECT_HEAD_INIT_WITH_BASECLASS("visibilityException", objectTypeException, OBJECT_TYPE_INSTANCE | OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE, &exception_funcs, &Object_Exception_CallException_struct), "", 0};
t_exception_object Object_Exception_ImportException_struct = { OBJECT_HEAD_INIT_WITH_BASECLASS("importException", objectTypeException, OBJECT_TYPE_INSTANCE | OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE, &exception_funcs, &Object_Exception_SystemException_struct), "", 0};
t_exception_object Object_Exception_ArithmeticException_struct = { OBJECT_HEAD_INIT_WITH_BASECLASS("arithmeticException", objectTypeException, OBJECT_TYPE_INSTANCE | OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE, &exception_funcs, &Object_Exception_struct), "", 0};
t_exception_object Object_Exception_DivideByZeroException_struct = { OBJECT_HEAD_INIT_WITH_BASECLASS("divideByZeroException", objectTypeException, OBJECT_TYPE_INSTANCE | OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE, &exception_funcs, &Object_Exception_ArithmeticException_struct), "", 0};
t_exception_object Object_Exception_IoException_struct = { OBJECT_HEAD_INIT_WITH_BASECLASS("ioException", objectTypeException, OBJECT_TYPE_INSTANCE | OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE, &exception_funcs, &Object_Exception_struct), "", 0};
t_exception_object Object_Exception_FileException_struct = { OBJECT_HEAD_INIT_WITH_BASECLASS("fileException", objectTypeException, OBJECT_TYPE_INSTANCE | OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE, &exception_funcs, &Object_Exception_IoException_struct), "", 0};
t_exception_object Object_Exception_FileNotFoundException_struct = { OBJECT_HEAD_INIT_WITH_BASECLASS("fileNotFoundException", objectTypeException, OBJECT_TYPE_INSTANCE | OBJECT_FLAG_STATIC | OBJECT_FLAG_IMMUTABLE, &exception_funcs, &Object_Exception_FileException_struct), "", 0};
