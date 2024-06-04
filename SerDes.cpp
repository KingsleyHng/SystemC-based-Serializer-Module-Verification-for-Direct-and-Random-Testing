
#include<systemc.h>
#include<cstdlib>
#include<ctime>

SC_MODULE(Serializer) {
    sc_in<bool> clk;
    sc_in<bool> reset;
    sc_in<sc_uint<8>> parallel_data;
    sc_out<bool> serial_data;
    sc_out<bool> ready;
    sc_out<sc_uint<8>> shift_reg;
    sc_out<int> bit_count;
    sc_out<bool> valid; // Changed to output

    sc_out<bool> load; // Internal load signal

    int cnt;

    void generate_load_signal() {
        // Add logic here to control the load signal
        if (reset.read() == true) {
            load.write(false);
        }
        else if (!ready.read() && !valid.read()) {
            load.write(true);
        }
        else {
            load.write(false);
        }
    }

    void process() {
        if (reset.read() == true) {
            shift_reg.write(0);
            bit_count.write(0);
            serial_data.write(0);
            ready.write(false);
            valid.write(false); // Initialize valid signal
            //load.write(0);
            cnt = 0;
        }
        else if (load.read() == true) {
            shift_reg.write(parallel_data.read());
            bit_count.write(7);
            ready.write(true);
            // Set valid to true on load
        }
        else if (ready.read() == true && bit_count.read() >= 0) {
            serial_data.write(shift_reg.read()[bit_count.read()]);

            valid.write(bit_count.read() > 0);
            bit_count.write(bit_count.read() - 1);
            valid.write(true);
            cnt++;
        }
        else if (cnt <= 9) {
            valid.write(false); // Ensure valid is low after 10 clock cycles
            ready.write(false);
            cnt = 0;
        }
    }

    SC_CTOR(Serializer) {
        SC_METHOD(process);
        sensitive << clk.pos();
        sensitive << reset;

        SC_METHOD(generate_load_signal);
        sensitive << clk.pos();
        sensitive << reset;
    }
};

// Testbench module definition
SC_MODULE(Testbench) {
    sc_signal<bool> clk;
    sc_signal<bool> reset;
    sc_signal<sc_uint<8>> parallel_data;
    sc_signal<bool> load;
    sc_signal<bool> serial_data;
    sc_signal<bool> ready;
    sc_signal<sc_uint<8>> shift_reg;
    sc_signal<int> bit_count;
    sc_signal<bool> valid;

    Serializer* serializer;

    void clock_gen() {
        while (true) {
            clk.write(false);
            wait(5, SC_NS);
            clk.write(true);
            wait(5, SC_NS);
        }
    }

    void stimulus() {
        /*
        // Initialize signals
        // Direct test
        reset.write(true);
        parallel_data.write(0x23);  //
        wait(10, SC_NS);  // Allow for reset
        reset.write(false);
        // Wait for serialization and deserialization
        wait(200, SC_NS);
        parallel_data.write(0x43);
        wait(200, SC_NS);//at least 115ns
        sc_stop();
        */

        //Random test
        reset.write(true);
        wait(10, SC_NS);  // Allow for reset
        reset.write(false);

        std::srand(std::time(0));
        cout << "time: " << std::time(0);

        // Loop to generate random inputs and run the simulation
        for (int i = 0; i < 5; ++i) {
            // Generate random values for inputs
            parallel_data = std::rand() % 256; // Random value between 0 and 255
            wait(200, SC_NS);

        }
        sc_stop();
    }

    SC_CTOR(Testbench) {
        serializer = new Serializer("serializer");
        serializer->clk(clk);
        serializer->reset(reset);
        serializer->parallel_data(parallel_data);
        serializer->load(load);
        serializer->serial_data(serial_data);
        serializer->ready(ready);
        serializer->shift_reg(shift_reg);
        serializer->bit_count(bit_count);
        serializer->valid(valid);

        SC_THREAD(clock_gen);
        SC_THREAD(stimulus);
    }

    ~Testbench() {
        delete serializer;
    }
};

// sc_main function to start simulation
int sc_main(int argc, char* argv[]) {
    Testbench tb("tb");
    sc_trace_file* vcdfile = sc_create_vcd_trace_file("waves");
    sc_trace(vcdfile, tb.clk, "clock");
    sc_trace(vcdfile, tb.reset, "reset");
    sc_trace(vcdfile, tb.parallel_data, "parallel_data");
    sc_trace(vcdfile, tb.load, "load");
    sc_trace(vcdfile, tb.serial_data, "serial_data");
    sc_trace(vcdfile, tb.ready, "ready");
    sc_trace(vcdfile, tb.valid, "valid");
    sc_trace(vcdfile, tb.bit_count, "bit_count");
    sc_trace(vcdfile, tb.shift_reg, "shift_reg");
    sc_start();  // Start the simulation

    sc_close_vcd_trace_file(vcdfile);  // Close the VCD trace file
    cout << "Simulation completed\n";
    return 0;
}
