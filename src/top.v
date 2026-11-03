module top (
    input  wire       clkin,
    input  wire       start_pin,
    input  wire       data_pin,
    input  wire       clk_pin,
    output wire [9:0] pwm_out
);

wire HIGHSPEEDclk;
wire [27:0] counter_out;
wire [9:0] sine_out;
wire [9:0] triangle_out;
wire [9:0] sawtooth_out;
wire [9:0] square_out;
wire [23:0] STEPSIZE;
wire [1:0] waveOut;


Gowin_rPLL pll_inst (
    .clkout(HIGHSPEEDclk),
    .clkin(clkin)
);

Counter counter_inst (
    .CLK(HIGHSPEEDclk),
    .OUTPUT(counter_out),
    .STEP(STEPSIZE)
);

shift_rx r_inst (
    .start_pin(start_pin),
    .data_pin(data_pin),
    .clk_pin(clk_pin),
    .freq_data(STEPSIZE),
    .wave_choice(waveOut)
);

sin sin_inst (
    .INPUT(counter_out[27:15]),
    .OUTPUT(sine_out)
);

triangle triangle_inst (
    .INPUT(counter_out[27:18]),
    .OUTPUT(triangle_out)
);

assign sawtooth_out = counter_out[27:18];
assign square_out = counter_out[27] ? 10'b1111111111 : 10'b0000000000; // Square wave

waveform_selector selector_inst (
    .select(waveOut),
    .sine_in(sine_out),
    .triangle_in(triangle_out),
    .sawtooth_in(sawtooth_out),
    .square_in(square_out),
    .waveform_out(pwm_out)
);

endmodule
