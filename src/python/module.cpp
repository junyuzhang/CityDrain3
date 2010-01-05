#include "module.h"
#include <noderegistry.h>
#include "pythonnodefactory.h"
#include "log.h"

#include "pymodel.h"
#include "pynode.h"
#include "pyflow.h"
#include "pysimulation.h"

#include <boost/python.hpp>
#include <string>

using namespace boost::python;


void init() {
	Log::init(&cout, Debug);
}

/**
  *log with default level to debug
  */
void logdebug(std::string msg) {
	Logger(Debug) << msg;
}

/**
  *log with level
  */
void logwithlevel(std::string msg, LogLevel logl) {
	Logger(logl) << msg;
}


BOOST_PYTHON_MODULE(pycd3) {
	docstring_options doc_options;
	doc_options.disable_cpp_signatures();
	scope().attr("__doc__") = "call pycd3.init() first then Flow.define()."
							  "not necessary if started from cd3";
	wrap_node();
	wrap_flow();
	wrap_model();
	wrap_simulation();
	def("init", ::init, "must be called first\n initializes the logger");
	def("log", logdebug);
	def("log", logwithlevel);
	enum_<LogLevel>("LogLevel")
		.value("debug", Debug)
		.value("standard", Standard)
		.value("error", Error)
		.value("warning", Warning)
		;
}

struct PythonEnvPriv {
	object main_module, main_namespace;
};

PythonEnv *PythonEnv::instance = 0;

PythonEnv::PythonEnv() {
	priv = new PythonEnvPriv();
	Py_Initialize();
	initpycd3();
	priv->main_module = import("__main__");
	priv->main_namespace = priv->main_module.attr("__dict__");
}

PythonEnv::~PythonEnv() {
	delete priv;
}

PythonEnv *PythonEnv::getInstance() {
	if (instance == 0) {
		instance = new PythonEnv();
	}

	return instance;
}

void PythonEnv::freeInstance() {
	if (instance == 0)
		return;
	exec("import gc\n"
		 "print gc.collect()\n"
		 "print 'where is the garbage'\n"
		 "print gc.garbage\n",
		 instance->priv->main_namespace,instance->priv->main_namespace);

	delete instance;
	Py_Finalize();
}

void PythonEnv::registerNodes(NodeRegistry *registry, const string &module) {
	format fmt("import sys\n"
	"import pycd3\n"
	"sys.path.append('./data/scripts/')\n"
	"__import__('%1%', None, None, [], 1)\n"
	"clss = pycd3.Node.__subclasses__()\n");

	fmt % module;

	try {
		object result = exec(fmt.str().c_str(),
							 priv->main_namespace,
							 priv->main_namespace);
		object clss = priv->main_namespace["clss"];
		int numn = 0;		
		for (int i = 0; i < len(clss); i++) {
			string name = extract<string>(clss[i].attr("__name__"));
			if (registry->contains(name))
				continue;
			registry->addNodeFactory(new PythonNodeFactory(clss[i]));
			numn++;
		}
		cout << "found " << numn << " Nodes in module " << module << endl;
	} catch(error_already_set const &) {
		cerr << __FILE__ << ":" << __LINE__ << endl;
		PyErr_Print();
		abort();
	}
}