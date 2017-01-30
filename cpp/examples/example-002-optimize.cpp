#include "neuralfield.hpp"
#include <iostream>

#include "rng_generators.h"
typedef popot::rng::CRNG RNG_GENERATOR;

#include "popot.h"
typedef popot::algorithm::ParticleStochasticSPSO::VECTOR_TYPE TVector;


using Input = std::vector<double>;

void fillInput(neuralfield::values_iterator begin,
	       neuralfield::values_iterator end,
	       const Input& x) {
  std::copy(x.begin(), x.end(), begin);
}

Input generate_input(std::vector<int> shape,
		     std::string label) {
  int size = 1;
  for(auto s: shape)
    size *= s;

  Input input(size);
  
  if(label == "random") {
    for(auto& v: input)
      v = neuralfield::random::uniform(0.0, 1.0);
  }
  else if(label == "structured") {
    // TODO
  }
  return input;
}

double evaluate(std::shared_ptr<neuralfield::Network> net, double * params) {

  // Set the parameters of the field
  // params = [dttau     h,  Ap,   sm, ka, ks]

  double dt_tau = params[0];
  double h = params[1];
  double Ap = params[2];
  double sm = params[3];
  double ka = params[4];
  double ks = params[5];
  
  double Am = ka * Ap;
  double sp = ks * sm;
  
  net->get("gexc")->set_parameters({Ap, sp});
  net->get("ginh")->set_parameters({Am, sm});
  net->get("h")->set_parameters({h});
  net->get("u")->set_parameters({dt_tau});
  
  // Test the net on the different scenarii
  /*
  Scenario s = CompetitionScenario("random");
  double f1 = s.evaluate(net);
  
  Scenario s2 = CompetitionScenario("structured");
  double f2 = s.evaluate(net);

  
  return f1+f2;
  */
  return 0;
}

int main(int argc, char * argv[]) {

  if(argc != 2) {
    std::cerr << "Script to optimize a 2D neural field for a competition scenario" << std::endl;
    std::cerr << "Usage : " << argv[0] << " N" << std::endl;
    std::exit(-1);
  }

  RNG_GENERATOR::rng_srand();
  RNG_GENERATOR::rng_warm_up();
  
  int N = atoi(argv[1]);

  double dt_tau = 0.01;
  double baseline = 0.0;
  double Ap = 1.5;
  double sp = 2.;
  double Am = -1.3;
  double sm = 10.;
  bool toric = false;
  unsigned int Nsteps = 100;
  
  // 1D
  std::initializer_list<int> shape({N});
  
  // 2D
  // std::initializer_list<int> shape({N, N});
  
  auto input = neuralfield::input::input<Input>(shape, fillInput, "input");

  auto h = neuralfield::function::constant(baseline, shape, "h");
  auto u = neuralfield::buffered::leaky_integrator(dt_tau, shape, "u");
  auto g_exc = neuralfield::link::gaussian(Ap, sp, toric, shape,"gexc");
  auto g_inh =  neuralfield::link::gaussian(Am, sm, toric, shape, "ginh");
  auto fu = neuralfield::function::function("sigmoid", shape, "fu");

  g_exc->connect(fu);
  g_inh->connect(fu);
  fu->connect(u);
  u->connect(g_exc + g_inh + input + h);
  
  auto net = neuralfield::get_current_network();
  net->print();

  net->init();

  //// This is how one scenario should goes on
  net->get("gexc")->set_parameters({1.4, 3.0});
  net->reset();

  auto I = generate_input(shape, "random");
  net->set_input<Input>("input", I);
  
  for(unsigned int i = 0 ; i < Nsteps; ++i)
    net->step();
  ////
  

  // Parametrization of popot

  const unsigned int Nparams = 6;
  //                                  dttau     h,  Ap,   sm, ka, ks 
  std::array<double, Nparams> lbounds({0.01, -1.0, 0.0,  1.0, 0., 0.});
  std::array<double, Nparams> ubounds({1.00,  1.0, 1.0,  double(N), 1., 1.});
  auto lbound = [lbounds] (size_t index) -> double { return lbounds[index];};
  auto ubound = [ubounds] (size_t index) -> double { return ubounds[index];};
  
  auto stop =   [] (double fitness, int epoch) -> bool { return epoch >= 1000 || fitness <= 0.001;};
  
  auto cost_function = [net] (TVector& pos) -> double { 
    return evaluate(net, pos.getValuesPtr());
  };
  
  auto algo = popot::algorithm::stochastic_montecarlo_spso2006(Nparams, 
							       lbound, 
							       ubound, 
							       stop, 
							       cost_function, 
							       10);
    
  // We run the algorithm
  algo->run(1);
  
  std::cout << "Best particle :" << algo->getBest() << std::endl;



  return 0;
}
