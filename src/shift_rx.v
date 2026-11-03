// Simple SPI receiver (Mode 0, MSB first) for the Arduino link.
// Frame: 32 bits while start_pin (CS) is low.
//   bits [25:2] -> freq_data   (24-bit phase step)
//   bits [1:0]  -> wave_choice (00 square, 01 triangle, 10 sawtooth, 11 sine)
// The top 6 bits of the frame are ignored (they shift out the top).
module shift_rx (
    input  wire        start_pin,   // CS, active low
    input  wire        data_pin,    // MOSI
    input  wire        clk_pin,     // SCLK
    output reg  [23:0] freq_data   = 24'd0,
    output reg  [1:0]  wave_choice = 2'd0
);

reg [25:0] shift = 26'd0;

// shift in one bit on every rising SCLK edge (top bit falls off the end)
always @(posedge clk_pin)
    shift <= {shift, data_pin};

// when CS goes high the frame is done: latch the new settings
always @(posedge start_pin) begin
    freq_data   <= shift[25:2];
    wave_choice <= shift[1:0];
end

endmodule
