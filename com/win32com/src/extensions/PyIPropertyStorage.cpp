// This file implements the IPropertyStorage Interface and Gateway for Python.
// Generated by makegw.py

#include "stdafx.h"
#include "PythonCOM.h"

#ifndef NO_PYCOM_IPROPERTYSTORAGE
#include "PyIPropertyStorage.h"

// @doc - This file contains autoduck documentation
// ---------------------------------------------------

// @object PROPSPEC|Identifies a property.  Can be either an int property id, or a str/unicode property name.
BOOL PyObject_AsPROPSPECs( PyObject *ob, PROPSPEC **ppRet, ULONG *pcRet)
{
	BOOL ret=FALSE;
	DWORD len, i;
	PyObject *tuple=PyWinSequence_Tuple(ob, &len);
	if (tuple==NULL)
		return FALSE;

	// First count the items, and the total string space we need.
	size_t cChars = 0;
	for (i=0;i<len;i++) {
		PyObject *sub = PyTuple_GET_ITEM(tuple, i);
		if (PyUnicode_Check(sub))
			cChars += PyUnicode_GET_SIZE(sub) + 1;
		else if (PyString_Check(sub))
			cChars += PyString_Size(sub) + 1;
		else if (PyInt_Check(sub))
			;	// PROPID is a ULONG, so this may fail for values that require a python long
		else {
			PyErr_SetString(PyExc_TypeError, "PROPSPECs must be a sequence of strings or integers");
			goto cleanup;
		}
	}
	size_t numBytes;
	numBytes = (sizeof(PROPSPEC) * len) + (sizeof(WCHAR) * cChars);
	PROPSPEC *pRet;
	pRet = (PROPSPEC *)malloc(numBytes);
	if (pRet==NULL) {
		PyErr_SetString(PyExc_MemoryError, "allocating PROPSPECs");
		goto cleanup;
	}
	WCHAR *curBuf;
	curBuf = (WCHAR *)(pRet+len);
	for (i=0;i<len;i++) {
		PyObject *sub = PyTuple_GET_ITEM(tuple, i);
		BSTR bstr;
		if (PyWinObject_AsBstr(sub, &bstr)) {
			pRet[i].ulKind = PRSPEC_LPWSTR;
			pRet[i].lpwstr = curBuf;
			wcscpy( curBuf, bstr);
			curBuf += wcslen(curBuf) + 1;
			PyWinObject_FreeBstr(bstr);
		} else {
			PyErr_Clear();
			pRet[i].ulKind = PRSPEC_PROPID;
			pRet[i].propid = PyInt_AsLong(sub);
		}
	}
	ret=TRUE;
	*ppRet = pRet;
	*pcRet = len;
cleanup:
	Py_DECREF(tuple);
	return ret;
}

void PyObject_FreePROPSPECs(PROPSPEC *pFree, ULONG /*cFree*/)
{
	if (pFree)
		free(pFree);
}

PyObject *PyObject_FromPROPVARIANT( PROPVARIANT *pVar )
{
	PyObject *ob;
	switch (pVar->vt) {
		case VT_EMPTY:
		case VT_NULL:
		case VT_ILLEGAL:
			Py_INCREF(Py_None);
			return Py_None;
		case VT_I1:
			return PyInt_FromLong(pVar->bVal);
		case VT_UI1:
			return PyInt_FromLong(pVar->bVal);
		case VT_I2:
			return PyInt_FromLong(pVar->iVal);
		case VT_UI2:
			return PyInt_FromLong(pVar->uiVal);
		case VT_I4:
			return PyInt_FromLong(pVar->lVal);
//		case VT_INT:
//			return PyInt_FromLong(pVar->intVal);
		case VT_UI4:
			return PyInt_FromLong(pVar->ulVal);
//		case VT_UINT:
//			return PyInt_FromLong(pVar->uintVal);
		case VT_I8:
			return PyWinObject_FromLARGE_INTEGER(pVar->hVal);
		case VT_UI8:
			return PyWinObject_FromULARGE_INTEGER(pVar->uhVal);
		case VT_R4:
			return PyFloat_FromDouble(pVar->fltVal);
		case VT_R8:
			return PyFloat_FromDouble(pVar->dblVal);
		case VT_CY:
			return PyObject_FromCurrency(pVar->cyVal);
		case VT_DATE:
			return PyWinObject_FromDATE(pVar->date);
		case VT_BSTR:
			return PyWinObject_FromBstr(pVar->bstrVal);
		case VT_BOOL:
			ob = pVar->boolVal ? Py_True : Py_False;
			Py_INCREF(ob);
			return ob;
		case VT_ERROR:
			return PyInt_FromLong(pVar->scode);
		case VT_FILETIME:
			return PyWinObject_FromFILETIME(pVar->filetime);
		case VT_LPSTR:
			if (pVar->pszVal == NULL) {
				Py_INCREF(Py_None);
				return Py_None;
			}
			return PyString_FromString(pVar->pszVal);
		case VT_LPSTR|VT_VECTOR:
			{
				PyObject *ret = PyList_New(pVar->calpstr.cElems);
				if (ret==NULL) return NULL;
				for (ULONG i=0; i<pVar->calpstr.cElems;i++){
					PyObject *elem=PyString_FromString(pVar->calpstr.pElems[i]);
					if (elem==NULL){
						Py_DECREF(ret);
						return NULL;
						}
					PyList_SET_ITEM(ret, i, elem);
					}
				return ret;
			}
		case VT_LPWSTR:
			return PyWinObject_FromOLECHAR(pVar->pwszVal);
		case VT_LPWSTR|VT_VECTOR:
			{
				PyObject *ret = PyList_New(pVar->calpwstr.cElems);
				if (ret==NULL) return NULL;
				for (ULONG i=0; i<pVar->calpwstr.cElems;i++){
					PyObject *elem=PyWinObject_FromWCHAR(pVar->calpwstr.pElems[i]);
					if (elem==NULL){
						Py_DECREF(ret);
						return NULL;
						}
					PyList_SET_ITEM(ret, i, elem);
					}
				return ret;
			}
		case VT_CLSID:
			return PyWinObject_FromIID(*pVar->puuid);
		case VT_STREAM:
		case VT_STREAMED_OBJECT:
			return PyCom_PyObjectFromIUnknown(pVar->pStream, IID_IStream, TRUE);
		case VT_STORAGE:
		case VT_STORED_OBJECT:
			return PyCom_PyObjectFromIUnknown(pVar->pStorage, IID_IStorage, TRUE);
		case VT_VECTOR | VT_VARIANT:
			return PyObject_FromPROPVARIANTs(pVar->capropvar.pElems, pVar->capropvar.cElems);

//		case VT_UNKNOWN:
//			return PyCom_PyObjectFromIUnknown(pVar->punkVal, IID_IUnknown, TRUE);
//		case VT_DISPATCH:
//			return PyCom_PyObjectFromIUnknown(pVar->pdispVal, IID_IDispatch, TRUE);

/*
// Want to get VT_CF and VT_BLOB working with a test case first!
		case VT_CF: { // special "clipboard format"
			// cbSize is the size of the buffer pointed to 
			// by pClipData, plus sizeof(ulClipFmt)
			// XXX - in that case, shouldn't we pass
			// pClipData + sizeof(DWORD) to Py_BuildValue??
			ULONG cb = CBPCLIPDATA(*pVar->pclipdata);
			return Py_BuildValue("is#",
			                     pVar->pclipdata->ulClipFmt,
			                     pVar->pclipdata->pClipData,
			                     (int)cb);
			}
		case VT_BLOB:
			// DWORD count of bytes, followed by that many bytes of data.
			// The byte count does not include the four bytes for the
			// length of the count itself; an empty blob member would
			// have a count of zero, followed by zero bytes.
			return PyString_FromStringAndSize((const char *)pVar->blob.pBlobData,
			                                  pVar->blob.cbSize);
*/
		default:
			PyErr_Format(PyExc_TypeError, "Unsupported property type 0x%x", pVar->vt);
			return NULL;
	}
}

PyObject *PyObject_FromPROPVARIANTs( PROPVARIANT *pVars, ULONG cVars )
{
	PyObject *ret = PyTuple_New(cVars);
	if (ret==NULL)
		return NULL;
	for (ULONG i=0;i<cVars;i++) {
		PyObject *sub = PyObject_FromPROPVARIANT(pVars+i);
		if (sub==NULL) {
			Py_DECREF(ret);
			return NULL;
		}
		PyTuple_SET_ITEM(ret, i, sub);
	}
	return ret;
}

BOOL PyObject_AsPROPVARIANT(PyObject *ob, PROPVARIANT *pVar)
{
	if (ob==Py_True) {
		pVar->boolVal = -1;
		pVar->vt = VT_BOOL;
	} else if (ob==Py_False) {
		pVar->boolVal = 0;
		pVar->vt = VT_BOOL;
	} else if (PyInt_Check(ob)) {
		pVar->lVal = PyInt_AsLong(ob);
		pVar->vt = VT_I4;
	} else if (PyFloat_Check(ob)) {
		pVar->dblVal = PyFloat_AsDouble(ob);
		pVar->vt = VT_R8;
	} else if (PyUnicode_Check(ob) || PyString_Check(ob)) {
		PyWinObject_AsBstr(ob, &pVar->bstrVal);
		pVar->vt = VT_BSTR;
	} else {
		PyErr_SetString(PyExc_TypeError, "Unsupported object for PROPVARIANT");
		return FALSE;
	}
	return TRUE;
}

void PyObject_FreePROPVARIANTs(PROPVARIANT *pVars, ULONG cVars)
{	
	if (pVars){
		for (ULONG i=0;i<cVars;i++)
			PropVariantClear(pVars+i);
		delete [] pVars;
		}
}

BOOL PyObject_AsPROPVARIANTs(PyObject *ob, PROPVARIANT **ppRet, ULONG *pcRet)
{
	BOOL ret=FALSE;
	DWORD len, i;
	PyObject *tuple=PyWinSequence_Tuple(ob, &len);
	if (tuple==NULL)
		return FALSE;

	PROPVARIANT *pRet = new PROPVARIANT[len];
	if (pRet==NULL){
		PyErr_NoMemory();
		goto cleanup;
		}
	for (i=0;i<len;i++)
		PropVariantInit(pRet+i);

	for (i=0;i<len;i++) {
		PyObject *sub = PyTuple_GET_ITEM(tuple, i);
		if (!PyObject_AsPROPVARIANT(sub, pRet+i))
			goto cleanup;
		}
	ret=TRUE;
cleanup:
	if (ret){
		*ppRet = pRet;
		*pcRet = len;
		}
	else if (pRet)
		PyObject_FreePROPVARIANTs(pRet, len);
	Py_DECREF(tuple);
	return ret;
}

BOOL PyObject_AsPROPIDs(PyObject *ob, PROPID **ppRet, ULONG *pcRet)
{
	// PROPID and DWORD are both unsigned long
	return PyWinObject_AsDWORDArray(ob, ppRet, pcRet, FALSE);
}

void PyObject_FreePROPIDs(PROPID *pFree, ULONG)
{
	if (pFree)
		free(pFree);
}

//
// Interface Implementation

PyIPropertyStorage::PyIPropertyStorage(IUnknown *pdisp):
	PyIUnknown(pdisp)
{
	ob_type = &type;
}

PyIPropertyStorage::~PyIPropertyStorage()
{
}

/* static */ IPropertyStorage *PyIPropertyStorage::GetI(PyObject *self)
{
	return (IPropertyStorage *)PyIUnknown::GetI(self);
}

// @pymethod |PyIPropertyStorage|ReadMultiple|Reads specified properties from the current property set.
PyObject *PyIPropertyStorage::ReadMultiple(PyObject *self, PyObject *args)
{
	IPropertyStorage *pIPS = GetI(self);
	if ( pIPS == NULL )
		return NULL;
	PyObject *props;
	// @pyparm (<o PROPSPEC>, ...)|props||Sequence of property IDs or names.
	if ( !PyArg_ParseTuple(args, "O:ReadMultiple", &props))
		return NULL;
	ULONG cProps;
	PROPSPEC *pProps;
	if (!PyObject_AsPROPSPECs( props, &pProps, &cProps))
		return NULL;
	PROPVARIANT *pPropVars = new PROPVARIANT[cProps];
	if (pPropVars==NULL) {
		PyObject_FreePROPSPECs(pProps, cProps);
		PyErr_SetString(PyExc_MemoryError, "allocating PROPVARIANTs");
		return NULL;
	}
	ULONG i;
	for (i=0;i<cProps;i++)
		PropVariantInit(pPropVars+i);

	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pIPS->ReadMultiple( cProps, pProps, pPropVars );
	PY_INTERFACE_POSTCALL;

	PyObject *rc;
	if ( FAILED(hr) )
		rc = PyCom_BuildPyException(hr, pIPS, IID_IPropertyStorage);
	else
		rc = PyObject_FromPROPVARIANTs(pPropVars, cProps);

	// Cleanup the property IDs.
	PyObject_FreePROPSPECs( pProps, cProps );
	// Cleanup the prop variants.
	for (i=0;i<cProps;i++) {
		PropVariantClear(pPropVars+i);
	}
	delete [] pPropVars;
	return rc;
}

// @pymethod |PyIPropertyStorage|WriteMultiple|Creates or modifies properties in the property set
PyObject *PyIPropertyStorage::WriteMultiple(PyObject *self, PyObject *args)
{
	PyObject *ret=NULL;
	PROPSPEC *pProps = NULL;
	PROPVARIANT *pVals = NULL;
	ULONG cProps, cVals;

	IPropertyStorage *pIPS = GetI(self);
	if ( pIPS == NULL )
		return NULL;
	PyObject *obProps;
	PyObject *obValues;
	long minId = 2;
	// @pyparm (<o PROPSPEC>, ...)|props||Sequence containing names or integer ids of properties to write
	// @pyparm (<o PROPVARIANT>, ...)|values||The values for the properties.
	// @pyparm int|propidNameFirst|2|Minimum property id to be assigned to new properties specified by name
	if ( !PyArg_ParseTuple(args, "OO|l:WriteMultiple", &obProps, &obValues, &minId))
		return NULL;
	
	if (!PyObject_AsPROPSPECs( obProps, &pProps, &cProps))
		goto cleanup;
	if (!PyObject_AsPROPVARIANTs( obValues, &pVals, &cVals ))
		goto cleanup;

	if (cProps != cVals) {
		PyErr_SetString(PyExc_ValueError, "The parameters must be sequences of the same size");
		goto cleanup;
	}

	HRESULT hr;
	{
	PY_INTERFACE_PRECALL;
	hr = pIPS->WriteMultiple( cProps, pProps, pVals, minId );
	PY_INTERFACE_POSTCALL;
	}
	if ( FAILED(hr) )
		PyCom_BuildPyException(hr, pIPS, IID_IPropertyStorage);
	else{
		Py_INCREF(Py_None);
		ret = Py_None;
		}

cleanup:
	PyObject_FreePROPSPECs(pProps, cProps);
	PyObject_FreePROPVARIANTs(pVals, cVals);
	return ret;
}

// @pymethod |PyIPropertyStorage|DeleteMultiple|Deletes properties from the property set
PyObject *PyIPropertyStorage::DeleteMultiple(PyObject *self, PyObject *args)
{
	IPropertyStorage *pIPS = GetI(self);
	if ( pIPS == NULL )
		return NULL;

	PyObject *props;
	// @pyparm (<o PROPSPEC>, ...)|props||Sequence containing names or IDs of properties to be deleted
	if ( !PyArg_ParseTuple(args, "O:ReadMultiple", &props))
		return NULL;
	ULONG cProps;
	PROPSPEC *pProps;
	if (!PyObject_AsPROPSPECs( props, &pProps, &cProps))
		return NULL;

	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pIPS->DeleteMultiple( cProps, pProps );
	PY_INTERFACE_POSTCALL;

	PyObject_FreePROPSPECs(pProps, cProps);

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pIPS, IID_IPropertyStorage);
	Py_INCREF(Py_None);
	return Py_None;

}

// @pymethod |PyIPropertyStorage|ReadPropertyNames|Retrieves any existing string names for the specified property identifiers.
PyObject *PyIPropertyStorage::ReadPropertyNames(PyObject *self, PyObject *args)
{
	IPropertyStorage *pIPS = GetI(self);
	if ( pIPS == NULL )
		return NULL;
	PyObject *obProps;
	// @pyparm (int, ...)|props||Sequence of ints containing property IDs.
	if ( !PyArg_ParseTuple(args, "O:ReadPropertyNames", &obProps))
		return NULL;

	ULONG cProps;
	PROPID *pProps;
	if (!PyObject_AsPROPIDs( obProps, &pProps, &cProps))
		return NULL;

	HRESULT hr;
	LPWSTR *ppStrs = new LPWSTR[cProps];
	if (ppStrs==NULL){
		PyErr_NoMemory();
		goto cleanup;
		}
	memset(ppStrs, 0, sizeof(LPWSTR)*cProps);
	{
	PY_INTERFACE_PRECALL;
	hr = pIPS->ReadPropertyNames( cProps, pProps, ppStrs );
	PY_INTERFACE_POSTCALL;
	}
	PyObject *rc;
	if ( FAILED(hr) )
		rc = PyCom_BuildPyException(hr, pIPS, IID_IPropertyStorage);
	else {
		rc = PyTuple_New(cProps);
		if (rc==NULL)
			goto cleanup;
		for (ULONG i=0;i<cProps;i++){
			PyObject *propname=PyWinObject_FromOLECHAR(ppStrs[i]);
			if (propname==NULL){
				Py_DECREF(rc);
				rc=NULL;
				goto cleanup;
				}
			PyTuple_SET_ITEM( rc, i, propname);
			}
		}

cleanup:
	if (ppStrs){
		for (ULONG i=0;i<cProps;i++)
			if (ppStrs[i])
				CoTaskMemFree(ppStrs[i]);
		delete [] ppStrs;
		}
	PyObject_FreePROPIDs(pProps, cProps);
	return rc;
}

// @pymethod |PyIPropertyStorage|WritePropertyNames|Assigns string names to a specified array of property IDs in the current property set.
PyObject *PyIPropertyStorage::WritePropertyNames(PyObject *self, PyObject *args)
{
	IPropertyStorage *pIPS = GetI(self);
	if ( pIPS == NULL )
		return NULL;
	PyObject *obProps;
	PyObject *obNames;
	// @pyparm (int, ...)|props||Sequence containing the property IDs.
	// @pyparm (string, ...)|names||Equal length sequence of property names.
	if ( !PyArg_ParseTuple(args, "OO:WritePropertyNames", &obProps, &obNames))
		return NULL;

	ULONG cProps = 0, cNames=0;
	PROPID *pProps = NULL;
	LPWSTR *ppStrs = NULL;
	PyObject *rc = NULL;

	if (!PyObject_AsPROPIDs( obProps, &pProps, &cProps))
		return NULL;
	if (!PyWinObject_AsWCHARArray(obNames, &ppStrs, &cNames, FALSE))
		goto done;
	if (cNames != cProps) {
		PyErr_SetString(PyExc_TypeError, "Property names must be a sequence the same size as property ids");
		goto done;
	}

	HRESULT hr;
	{
	PY_INTERFACE_PRECALL;
	hr = pIPS->WritePropertyNames( cProps, pProps, ppStrs );
	PY_INTERFACE_POSTCALL;
	}

	if ( FAILED(hr) )
		PyCom_BuildPyException(hr, pIPS, IID_IPropertyStorage);
	else{
		Py_INCREF(Py_None);
		rc = Py_None;
		}
done:
	PyObject_FreePROPIDs(pProps, cProps);
	PyWinObject_FreeWCHARArray(ppStrs, cNames);
	return rc;
}

// @pymethod |PyIPropertyStorage|DeletePropertyNames|Removes property names from specified properties.
PyObject *PyIPropertyStorage::DeletePropertyNames(PyObject *self, PyObject *args)
{
	IPropertyStorage *pIPS = GetI(self);
	if ( pIPS == NULL )
		return NULL;
	PyObject *obProps;
	// @pyparm (int, ...)|props||Sequence of ints containing property IDs.
	if ( !PyArg_ParseTuple(args, "O:DeletePropertyNames", &obProps))
		return NULL;

	ULONG cProps;
	PROPID *pProps;
	if (!PyObject_AsPROPIDs( obProps, &pProps, &cProps))
		return NULL;
	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pIPS->DeletePropertyNames( cProps, pProps );
	PY_INTERFACE_POSTCALL;
	PyObject_FreePROPIDs(pProps, cProps);
	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pIPS, IID_IPropertyStorage);
	Py_INCREF(Py_None);
	return Py_None;

}

// @pymethod |PyIPropertyStorage|Commit|Persists the property set to its base storage
PyObject *PyIPropertyStorage::Commit(PyObject *self, PyObject *args)
{
	IPropertyStorage *pIPS = GetI(self);
	if ( pIPS == NULL )
		return NULL;
	// @pyparm int|grfCommitFlags||Combination of STGC_* flags
	DWORD grfCommitFlags;
	if ( !PyArg_ParseTuple(args, "l:Commit", &grfCommitFlags) )
		return NULL;
	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pIPS->Commit( grfCommitFlags );
	PY_INTERFACE_POSTCALL;

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pIPS, IID_IPropertyStorage);
	Py_INCREF(Py_None);
	return Py_None;

}

// @pymethod |PyIPropertyStorage|Revert|Discards any changes that have been made
PyObject *PyIPropertyStorage::Revert(PyObject *self, PyObject *args)
{
	IPropertyStorage *pIPS = GetI(self);
	if ( pIPS == NULL )
		return NULL;
	if ( !PyArg_ParseTuple(args, ":Revert") )
		return NULL;
	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pIPS->Revert( );
	PY_INTERFACE_POSTCALL;

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pIPS, IID_IPropertyStorage);
	Py_INCREF(Py_None);
	return Py_None;

}

// @pymethod <o PyIEnumSTATPROPSTG>|PyIPropertyStorage|Enum|Creates an enumerator for properties in the property set
PyObject *PyIPropertyStorage::Enum(PyObject *self, PyObject *args)
{
	IPropertyStorage *pIPS = GetI(self);
	if ( pIPS == NULL )
		return NULL;
	IEnumSTATPROPSTG * ppenum;
	if ( !PyArg_ParseTuple(args, ":Enum") )
		return NULL;
	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pIPS->Enum( &ppenum );
	PY_INTERFACE_POSTCALL;

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pIPS, IID_IPropertyStorage);
	PyObject *obppenum;

	obppenum = PyCom_PyObjectFromIUnknown(ppenum, IID_IEnumSTATPROPSTG, FALSE);
	PyObject *pyretval = Py_BuildValue("O", obppenum);
	Py_XDECREF(obppenum);
	return pyretval;
}

// @pymethod |PyIPropertyStorage|SetTimes|Sets the creation, last access, and modification time
// @comm Some property sets do not support these times.
PyObject *PyIPropertyStorage::SetTimes(PyObject *self, PyObject *args)
{
	IPropertyStorage *pIPS = GetI(self);
	if ( pIPS == NULL )
		return NULL;
	// @pyparm <o PyTime>|pctime||Creation time
	// @pyparm <o PyTime>|patime||Last access time
	// @pyparm <o PyTime>|pmtime||Modification time
	PyObject *obpctime;
	PyObject *obpatime;
	PyObject *obpmtime;
	FILETIME pctime;
	FILETIME patime;
	FILETIME pmtime;
	if ( !PyArg_ParseTuple(args, "OOO:SetTimes", &obpctime, &obpatime, &obpmtime) )
		return NULL;
	BOOL bPythonIsHappy = TRUE;
	if (!PyWinObject_AsFILETIME(obpctime, &pctime))
		return NULL;
	if (!PyWinObject_AsFILETIME(obpatime, &patime))
		return NULL;
	if (!PyWinObject_AsFILETIME(obpmtime, &pmtime))
		return NULL;

	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pIPS->SetTimes( &pctime, &patime, &pmtime );
	PY_INTERFACE_POSTCALL;

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pIPS, IID_IPropertyStorage);
	Py_INCREF(Py_None);
	return Py_None;

}

// @pymethod |PyIPropertyStorage|SetClass|Sets the GUID for the property set
PyObject *PyIPropertyStorage::SetClass(PyObject *self, PyObject *args)
{
	IPropertyStorage *pIPS = GetI(self);
	if ( pIPS == NULL )
		return NULL;
	// @pyparm <o PyIID>|clsid||Description for clsid
	PyObject *obclsid;
	IID clsid;
	if ( !PyArg_ParseTuple(args, "O:SetClass", &obclsid) )
		return NULL;
	if (!PyWinObject_AsIID(obclsid, &clsid))
		return NULL;
	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pIPS->SetClass( clsid );
	PY_INTERFACE_POSTCALL;

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pIPS, IID_IPropertyStorage);
	Py_INCREF(Py_None);
	return Py_None;

}

// @pymethod |PyIPropertyStorage|Stat|Returns various infomation about the property set
PyObject *PyIPropertyStorage::Stat(PyObject *self, PyObject *args)
{
	IPropertyStorage *pIPS = GetI(self);
	if ( pIPS == NULL )
		return NULL;
	if ( !PyArg_ParseTuple(args, ":Stat") )
		return NULL;
	STATPROPSETSTG p;
	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pIPS->Stat( &p );
	PY_INTERFACE_POSTCALL;

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pIPS, IID_IPropertyStorage);
	return PyCom_PyObjectFromSTATPROPSETSTG(&p);
}

// @object PyIPropertyStorage|Description of the interface
static struct PyMethodDef PyIPropertyStorage_methods[] =
{
	{ "ReadMultiple", PyIPropertyStorage::ReadMultiple, 1 }, // @pymeth ReadMultiple|Reads specified properties from the current property set.
	{ "WriteMultiple", PyIPropertyStorage::WriteMultiple, 1 }, // @pymeth WriteMultiple|Creates or modifies properties in the property set
	{ "DeleteMultiple", PyIPropertyStorage::DeleteMultiple, 1 }, // @pymeth DeleteMultiple|Deletes properties from the property set
	{ "ReadPropertyNames", PyIPropertyStorage::ReadPropertyNames, 1 }, // @pymeth ReadPropertyNames|Retrieves any existing string names for the specified property identifiers.
	{ "WritePropertyNames", PyIPropertyStorage::WritePropertyNames, 1 }, // @pymeth WritePropertyNames|Assigns string names to a specified array of property IDs in the current property set.
	{ "DeletePropertyNames", PyIPropertyStorage::DeletePropertyNames, 1 }, // @pymeth DeletePropertyNames|Removes property names from specified properties.
	{ "Commit", PyIPropertyStorage::Commit, 1 }, // @pymeth Commit|Persists the property set to its base storage
	{ "Revert", PyIPropertyStorage::Revert, 1 }, // @pymeth Revert|Discards any changes that have been made
	{ "Enum", PyIPropertyStorage::Enum, 1 }, // @pymeth Enum|Creates an enumerator for properties in the property set
	{ "SetTimes", PyIPropertyStorage::SetTimes, 1 }, // @pymeth SetTimes|Sets the creation, last access, and modification time
	{ "SetClass", PyIPropertyStorage::SetClass, 1 }, // @pymeth SetClass|Sets the GUID for the property set
	{ "Stat", PyIPropertyStorage::Stat, 1 }, // @pymeth Stat|Returns various infomation about the property set
	{ NULL }
};

PyComEnumProviderTypeObject PyIPropertyStorage::type("PyIPropertyStorage",
		&PyIUnknown::type,
		sizeof(PyIPropertyStorage),
		PyIPropertyStorage_methods,
		GET_PYCOM_CTOR(PyIPropertyStorage),
		"Enum");

#endif // NO_PYCOM_IPROPERTYSTORAGE
