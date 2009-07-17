#ifndef MIXER_H
#define MIXER_H

#include <node.h>
#include <vector>
class Flow;

CD3_DECLARE_NODE(Mixer)
public:
	Mixer();
	~Mixer();
	int f(int time, int dt);
	void init(int start, int end, int dt);
	void deinit();
private:
	int num_inputs;
	Flow *out;
	std::vector<Flow*> inputs;
};

#endif // MIXER_H