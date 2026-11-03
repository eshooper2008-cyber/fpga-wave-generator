module Counter (
    input wire CLK,
    input wire [23:0] STEP,
    output reg [27:0] OUTPUT
    
);

always @(posedge CLK) begin  
    OUTPUT <= OUTPUT + STEP;
end

endmodule
