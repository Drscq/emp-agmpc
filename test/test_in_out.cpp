#include <emp-tool/emp-tool.h>
#include "emp-agmpc/emp-agmpc.h"

#include "emp-agmpc/flexible_input_output.h"

using namespace std;
using namespace emp;

const string filename = macro_xstr(EMP_CIRCUIT_PATH) + string("bristol_format/AES-non-expanded.txt");

const static int nP = 2;
int party, port;

void test_non_in_out(int party, int port) {
	cout << "Standard in/out without using FlexIn/FlexOut" << endl;
	cout << "compute: K = E_{010101...}(101010...); E_{010101...}(K)" << endl;
	PRG prg;
	bool delta[128];
	prg.random_bool(delta, 128);	
	NetIOMP<nP> io(party, port);
	NetIOMP<nP> io2(party, port+2*(nP+1)*(nP+1)+1);
	
	NetIOMP<nP> *ios[2] = {&io, &io2};
	ThreadPool pool(2*(nP-1)+2);
	
	BristolFormat cf(filename.c_str());
	
	CMPC<nP>* mpc = new CMPC<nP>(ios, &pool, party, &cf, delta);
	ios[0]->flush();
	ios[1]->flush();
	
	mpc->function_independent();
	ios[0]->flush();
	ios[1]->flush();
	
	mpc->function_dependent();
	ios[0]->flush();
	ios[1]->flush();
	
	bool *in = new bool[cf.n1+cf.n2];
	memset(in, false, cf.n1+cf.n2);
	bool *out = new bool[cf.n3];
	
	if(party == ALICE) {
		for (int i = 0; i < cf.n1; i++) {
			in[i] = i % 2 == 0;
		}
	} else if (party == BOB){
		for (int i = 0; i < cf.n2; i++) {
			in[cf.n1 + i] = i % 2 == 1;
		}
	}
	
	mpc->online(in, out);
	ios[0]->flush();
	ios[1]->flush();
	
	delete mpc;
	
	CMPC<nP>* mpc2 = new CMPC<nP>(ios, &pool, party, &cf, delta);
	ios[0]->flush();
	ios[1]->flush();
	
	mpc2->function_independent();
	ios[0]->flush();
	ios[1]->flush();
	
	mpc2->function_dependent();
	ios[0]->flush();
	ios[1]->flush();
	
	bool *in2 = new bool[cf.n1+cf.n2];
	memset(in2, false, cf.n1+cf.n2);
	bool *out2 = new bool[cf.n3];
	
	if(party == ALICE) {
		for (int i = 0; i < cf.n1; i++) {
			in2[i] = out[i];
		}
	} else if (party == BOB){
		for (int i = 0; i < cf.n2; i++) {
			in2[cf.n1 + i] = i % 2 == 1;
		}
	}
	
	mpc2->online(in2, out2);
	ios[0]->flush();
	ios[1]->flush();
	
	if(party == ALICE) {
		cout << "output:" << endl;
		for (int i = 0; i < cf.n3; i++) {
			cout << out2[i] << " ";
		}
		cout << endl;
	}
	
	delete mpc2;
}

void test_in_out(int party, int port) {
	cout << "FlexIn/FlexOut" << endl;
	cout << "compute: K = E_{010101...}(101010...); E_{010101...}(K)" << endl;
	PRG prg;
	bool delta[128];
	prg.random_bool(delta, 128);	

	NetIOMP<nP> io(party, port);
	NetIOMP<nP> io2(party, port+2*(nP+1)*(nP+1)+1);
	
	NetIOMP<nP> *ios[2] = {&io, &io2};
	ThreadPool pool(2*(nP-1)+2);
	
	BristolFormat cf(filename.c_str());
	std::cout << "n1: " << cf.n1 << ", n2: " << cf.n2 << ", n3: " << cf.n3 << endl;
	
	CMPC<nP>* mpc = new CMPC<nP>(ios, &pool, party, &cf, delta);
	ios[0]->flush();
	ios[1]->flush();
	
	mpc->function_independent();
	ios[0]->flush();
	ios[1]->flush();
	
	mpc->function_dependent();
	ios[0]->flush();
	ios[1]->flush();
	
	FlexIn<nP> in(cf.n1 + cf.n2, party);
	for(int i = 0; i < 64; i++) {
		in.assign_party(i, ALICE);
	}
	for(int i = 64; i < cf.n1; i++) {
		in.assign_party(i, -2);
	}
	for(int i = 0; i < 64; i++) {
		in.assign_party(cf.n1 + i, 0);
	}
	for(int i = 64; i < cf.n2; i++) {
		in.assign_party(cf.n1 + i, BOB);
	}
	
	FlexOut<nP> out(cf.n3, party);
	for(int i = 0; i < cf.n3; i++) {
		out.assign_party(i, -1);
	}
	
	if(party == ALICE) {
		for(int i = 0; i < 64; i++){
			in.assign_plaintext_bit(i, i % 2 == 0);
		}
		for(int i = 64; i < cf.n1; i++){
			in.assign_plaintext_bit(i, i % 2 == 0);
		}
		for(int i = 0; i < 64; i++) {
			in.assign_plaintext_bit(cf.n1 + i, i % 2 == 1);
		}
	} else {
		for(int i = 64; i < cf.n1; i++){
			in.assign_plaintext_bit(i, false);
		}
		for(int i = 0; i < 64; i++) {
			in.assign_plaintext_bit(cf.n1 + i, i % 2 == 1);
		}
		for(int i = 64; i < cf.n2; i++) {
			in.assign_plaintext_bit(cf.n1 + i, i % 2 == 1);
		}
	}
	
	mpc->online(&in, &out);
	ios[0]->flush();
	ios[1]->flush();
	
	CMPC<nP>* mpc2 = new CMPC<nP>(ios, &pool, party, &cf, delta);
	ios[0]->flush();
	ios[1]->flush();
	
	mpc2->function_independent();
	ios[0]->flush();
	ios[1]->flush();
	
	mpc2->function_dependent();
	ios[0]->flush();
	ios[1]->flush();
	
	FlexIn<nP> in2(cf.n1 + cf.n2, party);
	for(int i = 0; i < cf.n1; i++) {
		in2.assign_party(i, -1);
	}
	for(int i = cf.n1; i < cf.n1 + cf.n2; i++) {
		in2.assign_party(i, BOB);
	}
	
	FlexOut<nP> out2(cf.n3, party);
	for(int i = 0; i < 32; i++) {
		out2.assign_party(i, ALICE);
	}
	for(int i = 32; i < 64; i++) {
		out2.assign_party(i, BOB);
	}
	for(int i = 64; i < cf.n3; i++) {
		out2.assign_party(i, 0);
	}
	
	for(int i = 0; i < cf.n1; i++) {
		AuthBitShare<nP> mabit = out.get_authenticated_bitshare(i);
		// if (party == ALICE) {
		// 	mabit.bit_share = not mabit.bit_share; // Alice's input is inverted
		// }
		in2.assign_authenticated_bitshare(i, &mabit);
	}
	
	if(party == BOB) {
		for(int i = 0; i < cf.n2; i++) {
			in2.assign_plaintext_bit(cf.n1 + i, i % 2 == 1);
		}
	}
	
	mpc2->online(&in2, &out2);
	ios[0]->flush();
	ios[1]->flush();
	
	cout << "output:" << endl;
	if(party == ALICE) {
		for (int i = 0; i < 32; i++) {
			cout << out2.get_plaintext_bit(i) << " ";
		}
		for (int i = 32; i < 64; i++) {
			cout << "x" << " ";
		}
		for (int i = 64; i < cf.n3; i++) {
			cout << out2.get_plaintext_bit(i) << " ";
		}
	} else {
		for (int i = 0; i < 32; i++) {
			cout << "x" << " ";
		}
		for (int i = 32; i < 64; i++) {
			cout << out2.get_plaintext_bit(i) << " ";
		}
		for (int i = 64; i < cf.n3; i++) {
			cout << out2.get_plaintext_bit(i) << " ";
		}
	}
	cout << endl;
	
	delete mpc;
	delete mpc2;
}

void test_authenticated_bitshares(int party, int port) {
	cout << "Authenticated BitShares test" << endl;
	cout << "Generate authenticated bitshares for all parties and use them in computation" << endl;
	PRG prg;
	bool delta[128];
	prg.random_bool(delta, 128);	

	NetIOMP<nP> io(party, port);
	NetIOMP<nP> io2(party, port+2*(nP+1)*(nP+1)+1);
	
	NetIOMP<nP> *ios[2] = {&io, &io2};
	ThreadPool pool(2*(nP-1)+2);
	
	BristolFormat cf(filename.c_str());
	std::cout << "n1: " << cf.n1 << ", n2: " << cf.n2 << ", n3: " << cf.n3 << endl;
	
	// Step 1: Generate authenticated bitshares using ABitMP
	ABitMP<nP>* abit_gen = new ABitMP<nP>(&io2, &pool, party, delta);
	
	// Generate authenticated bitshares for the input size
	int total_input_bits = cf.n1 + cf.n2;
	block* input_mac[nP+1];
	block* input_key[nP+1];
	bool* input_data = new bool[total_input_bits];
	
	// Initialize input data with some pattern
	for(int i = 0; i < total_input_bits; i++) {
		if(party == ALICE && i < cf.n1) {
			input_data[i] = i % 2 == 0;  // Alice's input pattern
		} else if(party == BOB && i >= cf.n1) {
			input_data[i] = i % 2 == 1;  // Bob's input pattern  
		} else {
			input_data[i] = false;  // Default for other bits
		}
	}
	
	// Allocate memory for MACs and KEYs
	for(int i = 1; i <= nP; i++) {
		input_mac[i] = new block[total_input_bits];
		input_key[i] = new block[total_input_bits];
	}
	
	// Generate authenticated bitshares
	abit_gen->compute(input_mac, input_key, input_data, total_input_bits);
	ios[0]->flush();
	ios[1]->flush();
	
	cout << "Generated " << total_input_bits << " authenticated bitshares" << endl;
	
	// Step 2: Create MPC instance for the actual computation
	CMPC<nP>* mpc = new CMPC<nP>(ios, &pool, party, &cf, delta);
	ios[0]->flush();
	ios[1]->flush();
	
	mpc->function_independent();
	ios[0]->flush();
	ios[1]->flush();
	
	mpc->function_dependent();
	ios[0]->flush();
	ios[1]->flush();
	
	// Step 3: Create FlexIn with authenticated bitshares
	FlexIn<nP> in(cf.n1 + cf.n2, party);
	
	// Assign all inputs as authenticated bitshares (-1)
	for(int i = 0; i < cf.n1 + cf.n2; i++) {
		in.assign_party(i, -1);
	}
	
	// Create AuthBitShare structures from the generated authenticated bitshares
	for(int i = 0; i < cf.n1 + cf.n2; i++) {
		AuthBitShare<nP> abit;
		abit.bit_share = input_data[i];
		
		for(int j = 1; j <= nP; j++) {
			if(j != party) {
				abit.key[j] = input_key[j][i];
				abit.mac[j] = input_mac[j][i];
			}
		}
		
		in.assign_authenticated_bitshare(i, &abit);
	}
	
	// Step 4: Create FlexOut with authenticated bitshares for outputs
	FlexOut<nP> out(cf.n3, party);
	
	// Assign all outputs as authenticated bitshares (-1)
	for(int i = 0; i < cf.n3; i++) {
		out.assign_party(i, -1);
	}
	
	// Step 5: Perform the computation
	mpc->online(&in, &out);
	ios[0]->flush();
	ios[1]->flush();
	
	cout << "Computation completed with authenticated bitshares" << endl;
	
	// Step 6: Extract the output authenticated bitshares
	cout << "Output authenticated bitshares:" << endl;
	for(int i = 0; i < min(10, cf.n3); i++) {  // Show first 10 outputs
		AuthBitShare<nP> output_abit = out.get_authenticated_bitshare(i);
		cout << "Output bit " << i << ": value = " << output_abit.bit_share << endl;
		
		// You can now use these authenticated bitshares in subsequent computations
		// The output_abit contains the bit_share, key[], and mac[] for all parties
	}
	
	// Step 7: Demonstrate using output authenticated bitshares as input to another computation
	cout << "Using output authenticated bitshares as input to second computation..." << endl;
	
	CMPC<nP>* mpc2 = new CMPC<nP>(ios, &pool, party, &cf, delta);
	ios[0]->flush();
	ios[1]->flush();
	
	mpc2->function_independent();
	ios[0]->flush();
	ios[1]->flush();
	
	mpc2->function_dependent();
	ios[0]->flush();
	ios[1]->flush();
	
	FlexIn<nP> in2(cf.n1 + cf.n2, party);
	
	// Use output authenticated bitshares as input for the first cf.n1 bits
	for(int i = 0; i < min(cf.n1, cf.n3); i++) {
		in2.assign_party(i, -1);
		AuthBitShare<nP> output_abit = out.get_authenticated_bitshare(i);
		in2.assign_authenticated_bitshare(i, &output_abit);
	}
	
	// For remaining inputs, use regular input assignment
	for(int i = cf.n1; i < cf.n1 + cf.n2; i++) {
		in2.assign_party(i, BOB);
	}
	
	if(party == BOB) {
		for(int i = cf.n1; i < cf.n1 + cf.n2; i++) {
			in2.assign_plaintext_bit(i, i % 2 == 1);
		}
	}
	
	FlexOut<nP> out2(cf.n3, party);
	for(int i = 0; i < cf.n3; i++) {
		out2.assign_party(i, -1);  // All outputs as authenticated bitshares
	}
	
	mpc2->online(&in2, &out2);
	ios[0]->flush();
	ios[1]->flush();
	
	cout << "Second computation completed!" << endl;
	cout << "Final authenticated bitshares generated for all " << cf.n3 << " output bits" << endl;
	
	// Cleanup
	delete abit_gen;
	delete mpc;
	delete mpc2;
	delete[] input_data;
	for(int i = 1; i <= nP; i++) {
		delete[] input_mac[i];
		delete[] input_key[i];
	}
}

int main(int argc, char** argv) {
	parse_party_and_port(argv, &party, &port);
	if(party > nP)return 0;
	
	// test_non_in_out(party, port);
	
	// cout << "============================" << endl;
	
	test_in_out(party, port);
	
	// cout << "============================" << endl;
	
	// test_authenticated_bitshares(party, port);
	
	return 0;
}