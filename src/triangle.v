module triangle (
    input wire [9:0] INPUT,
    output reg [9:0] OUTPUT
);

always @(*) begin
   if (INPUT[9]) begin
       OUTPUT = ~INPUT[8:0] << 1;
   end else begin
       OUTPUT = INPUT[8:0] << 1;
   end
end

endmodule
