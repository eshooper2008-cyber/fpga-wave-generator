module waveform_selector (
    input wire [1:0] select,
    input wire [9:0] sine_in,
    input wire [9:0] triangle_in,
    input wire [9:0] sawtooth_in,
    input wire [9:0] square_in,
    output reg [9:0] waveform_out
);

always @(*) begin
    case (select)
        2'b00: waveform_out = square_in;   
        2'b01: waveform_out = triangle_in;  
        2'b10: waveform_out = sawtooth_in;  
        2'b11: waveform_out = sine_in;   
        default: waveform_out = sine_in;
    endcase
end

endmodule
