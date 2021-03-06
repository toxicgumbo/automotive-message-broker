/*
    Copyright (C) 2012  Intel Corporation

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef DATABASESINK_H
#define DATABASESINK_H

#include "abstractsink.h"
#include "abstractsource.h"
#include "basedb.hpp"

#include <glib.h>

#define DatabaseLoggingProperty "DatabaseLogging"

template <typename T>
class Queue
{
public:
	Queue()
	{
		mutex = g_mutex_new();
	}

	int count()
	{
		g_mutex_lock(mutex);
		int ret = mQueue.count();
		g_mutex_unlock(mutex);

		return ret;
	}

	T pop()
	{
		g_mutex_lock(mutex);

		while(!mQueue.size())
		{
			g_cond_wait(&cond, mutex);
		}

		auto itr = mQueue.begin();

		T item = *itr;

		mQueue.erase(itr);

		g_mutex_unlock(mutex);

		return item;
	}

	void append(T item)
	{
		g_mutex_lock(mutex);

		g_cond_signal(&cond);

		mQueue.push_back(item);

		g_mutex_unlock(mutex);
	}

private:
	GMutex * mutex;
	GCond cond;
	std::vector<T> mQueue;
};

class DBObject {
public:
	DBObject(): time(0), sequence(0), quit(false) {}
	std::string key;
	std::string value;
	std::string source;
	double time;
	int32_t sequence;
	bool quit;
};

class Shared
{
public:
	Shared()
	{
		db = new BaseDB;
	}
	~Shared()
	{
		delete db;
	}

	BaseDB * db;
	Queue<DBObject*> queue;
};

class DatabaseSink : public AbstractSource
{

public:
	DatabaseSink(AbstractRoutingEngine* engine, map<string, string> config);
	~DatabaseSink();
	virtual void supportedChanged(PropertyList supportedProperties);
	virtual void propertyChanged(VehicleProperty::Property property, AbstractPropertyType* value, std::string uuid);
	virtual std::string uuid();

	///source role:
	virtual void getPropertyAsync(AsyncPropertyReply *reply);
	virtual void getRangePropertyAsync(AsyncRangePropertyReply *reply);
	virtual AsyncPropertyReply * setProperty(AsyncSetPropertyRequest request);
	virtual void subscribeToPropertyChanges(VehicleProperty::Property property);
	virtual void unsubscribeToPropertyChanges(VehicleProperty::Property property);
	virtual PropertyList supported();
	int supportedOperations() { return GetRanged; }

private: //methods:

	void parseConfig();

private:
	PropertyList mSubscriptions;
	Shared *shared;
	GThread* thread;
	std::string databaseName;
	std::string tablename;
	std::string tablecreate;
	std::list<VehicleProperty::Property> propertiesToSubscribeTo;
};

class DatabaseSinkManager: public AbstractSinkManager
{
public:
	DatabaseSinkManager(AbstractRoutingEngine* engine, map<string, string> config)
	:AbstractSinkManager(engine, config)
	{
		new DatabaseSink(routingEngine, config);
		VehicleProperty::registerProperty(DatabaseLoggingProperty, [](){return new BasicPropertyType<bool>(false);});
	}
};

#endif // DATABASESINK_H
