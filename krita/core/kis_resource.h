/*
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#if !defined KIS_RESOURCE_H_
#define KIS_RESOURCE_H_

#include <qimage.h>
#include <qobject.h>
#include <qstring.h>

/**
 * The KisResource class provides a representation of Krita image resources.  This
 * includes, but not limited to, brushes and patterns.
 *
 * This replaces the KisKrayon facility that used to be present in Krayon. 
 */
class KisResource : public QObject {
	typedef QObject super;
	Q_OBJECT

public:
	/**
	 * Creates a new KisResource object using @p filename.  No file is opened
	 * in the construtor, you have to call loadAsync.
	 *
	 * @param filename the file name to save and load from.  This isn't a complete path.
	 */
	KisResource(const QString& filename);
	virtual ~KisResource();

public:
	virtual bool loadAsync() = 0;
	virtual bool saveAsync() = 0;
	virtual QImage img() const = 0;
	virtual QImage frame(Q_INT32 n) const = 0;

public:
	virtual void cycle();
	virtual void resetCycle();

public:
	bool dirty() const;
	void setDirty(bool dirt);
	QString filename() const;
	void setFilename(const QString& filename);
	QString name() const;
	void setName(const QString& name);
	bool valid() const;
	void setValid(bool valid);
	Q_INT32 width() const;
	Q_INT32 height() const;

protected:
	void setWidth(Q_INT32 w);
	void setHeight(Q_INT32 h);

signals:
	void loadComplete(KisResource *me);
	void saveComplete(KisResource *me);

private:
	KisResource(const KisResource&);
	KisResource& operator=(const KisResource&);

private:
	QString m_name;
	QString m_filename;
	bool m_dirty;
	bool m_valid;
	Q_INT32 m_width;
	Q_INT32 m_height;
};

#endif // KIS_RESOURCE_H_

