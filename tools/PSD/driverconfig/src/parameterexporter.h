#ifndef PARAMETEREXPORTER_H
#define PARAMETEREXPORTER_H
#include <QFile>

class ParameterExporter
{
public:
	ParameterExporter(const QString &filename);
	~ParameterExporter();

	bool openExportFile();
	bool exportParameter();
private:
	bool write(const char *data, qint64 size);

	QFile m_file;
};

#endif // PARAMETEREXPORTER_H
