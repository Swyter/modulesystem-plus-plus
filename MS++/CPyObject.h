#include "Python.h"
#include <string>

class CPyIter;
class CPyTuple;
class CPyList;
class CPyString;
class CPyLong;
class CPyFloat;

class CPyObject
{
public:
	CPyObject();
	CPyObject(PyObject *obj);
	CPyObject(const CPyObject &cobj);
	~CPyObject();
	CPyObject Call(const CPyTuple &args) const;
	const CPyObject GetAttr(const CPyString &name) const;
	bool SetAttr(const CPyString &name, const CPyObject &value);
	bool DelAttr(const CPyString &name);
	const CPyObject GetItem(ssize_t pos) const;
	ssize_t Len() const;
	CPyString Str() const;
	CPyObject Type() const;
	CPyIter GetIter() const;
	CPyTuple AsTuple() const;
	CPyList AsList() const;
	CPyString AsString() const;
	CPyLong AsLong() const;
	CPyFloat AsFloat() const;
	bool IsTuple() const;
	bool IsList() const;
	bool IsString() const;
	bool IsLong() const;
	bool IsFloat() const;
	PyObject *GetRawObject() const;
	const CPyObject &operator =(const CPyObject& cobj);
	bool operator ==(const CPyObject& cobj) const;
	bool operator !=(const CPyObject& cobj) const;
	operator CPyTuple() const;
	operator CPyList() const;
	operator CPyString() const;
	operator CPyLong() const;
	operator CPyFloat() const;
	friend std::ostream & operator <<(std::ostream &stream, const CPyObject &cobj);

protected:
	int Check2(int val) const;
	int Check3(int val) const;
	PyObject *CheckObj(PyObject *obj) const;
	void CheckErr() const;

private:
	void SetObject(PyObject *obj);
	void CopyObject(PyObject *obj);

protected:
	PyObject *m_obj;
};

class CPyException
{
public:
	CPyException(const CPyString &text);
	const std::string &GetText() const;

private:
	std::string m_text;
};

class CPyModule : public CPyObject
{
public:
	CPyModule(const CPyString &name);
	CPyModule(PyObject *obj);
};

class CPyIter : public CPyObject
{
public:
	CPyIter(PyObject *obj);
	bool HasNext() const;
	CPyObject Next();

private:
	PyObject *m_next;
};

class CPyTuple : public CPyObject
{
public:
	CPyTuple();
	CPyTuple(ssize_t size);
	CPyTuple(PyObject *obj);
	CPyObject GetSlice(ssize_t low, ssize_t high) const;
	CPyObject GetItem(ssize_t pos) const;
	bool SetItem(ssize_t pos, const CPyObject& cobj);
	ssize_t Size() const;
};

class CPyList : public CPyObject
{
public:
	CPyList();
	CPyList(ssize_t size);
	CPyList(PyObject *obj);
	CPyObject GetSlice(ssize_t low, ssize_t high) const;
	CPyObject GetItem(ssize_t pos) const;
	bool SetItem(ssize_t pos, const CPyObject& cobj);
	ssize_t Size() const;
};

class CPyString : public CPyObject
{
public:
	CPyString();
	CPyString(const char *str);
	CPyString(const std::string &str);
	CPyString(PyObject *obj);
	operator std::string() const;
};

class CPyLong : public CPyObject
{
public:
	CPyLong();
	CPyLong(long long val);
	CPyLong(unsigned long long val);
	CPyLong(PyObject *obj);
	operator long() const;
	operator long long() const;
	operator unsigned long() const;
	operator unsigned long long() const;
};

class CPyFloat : public CPyObject
{
public:
	CPyFloat();
	CPyFloat(double val);
	CPyFloat(PyObject *obj);
	operator double() const;
};