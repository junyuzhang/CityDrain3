#ifndef SIMULATIONSCENE_H
#define SIMULATIONSCENE_H

#include <QGraphicsScene>
#include <QMap>
#include <string>
#include <nodeitem.h> //for PortType

class NodeRegistry;
class SimulationRegistry;
class MapBasedModel;
class ISimulation;
class QTreeWidget;
class PortItem;
class ConnectionItem;

class SimulationScene : public QGraphicsScene
{
Q_OBJECT
public:
	SimulationScene(QString model_file_name = "", QObject *parent = 0);
	virtual ~SimulationScene();
	void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
	void dropEvent(QGraphicsSceneDragDropEvent *event);
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

	void save();

	const NodeRegistry *getNodeRegistry() const { return node_reg; }
	SimulationRegistry *getSimulationRegistry() const { return sim_reg; }
	ISimulation *getSimulation() const { return simulation; }
	void setSimulation(ISimulation *simulation);
	MapBasedModel *getModel() const { return model; }
	QList<NodeItem*> getNodeItems() const { return node_items; }

	QString getModelFileName() const { return model_file_name; }
	void setModelFileName(QString name) { model_file_name = name; }
	void addPlugin(QString pname);
	void addPythonModule(QString pname);

	void remove(NodeItem *item);
	void remove(ConnectionItem *item);

private Q_SLOTS:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
	void load();
	bool isInPort(QGraphicsItem *item) const;
	bool isOutPort(QGraphicsItem *item) const;
	NodeRegistry *node_reg;
	SimulationRegistry *sim_reg;
	ISimulation *simulation;

	QMap<std::string, int> ids;
	MapBasedModel *model;
	PortItem *connection_start;
	ConnectionItem *current_connection;
	QList<NodeItem*> node_items;
	QList<ConnectionItem *> connection_items;
	QString model_file_name;
	QStringList plugins, python_modules;
};

#endif // SIMULATIONSCENE_H
