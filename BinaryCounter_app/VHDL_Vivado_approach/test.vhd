library ieee;
use ieee.std_logic_1164.all;

entity tb_main is
end tb_main;

architecture tb of tb_main is

    component main
        port (CLK : in std_logic;
              RST : in std_logic;
              LED : out std_logic_vector(5 downto 0);

              btn_signal : in std_logic
              );
    end component;

    signal CLK : std_logic;
    signal RST : std_logic;
    signal btn_signal : std_logic;
    signal LED : std_logic_vector(5 downto 0);

    constant TbPeriod : time := 10 ns; -- 100 MHz
    signal TbClock : std_logic := '0';
    signal TbSimEnded : std_logic := '0';

begin

    dut : main port map (CLK => CLK, RST => RST, btn_signal => btn_signal, LED => LED);

    -- Clock generation
    TbClock <= not TbClock after TbPeriod/2 when TbSimEnded /= '1' else '0';

    CLK <= TbClock;

    stimuli : process   
    begin
        
        --RST and button ext signal synchronization
        RST <= '1';
        wait for 50 ns;
        RST <= '0';
        wait for 50 ns;
        btn_signal <= '1';
        wait for 10 ns;
        btn_signal <= '0';
        
        wait for 70 ms; -- lets observe a few cycles of LED behaviour in S0 -> 50 Hz
        
        --BTN debouncing test 
        btn_signal <= '1';
        wait for 2 ms;
        btn_signal <= '0';
        wait for 1 ms;
        btn_signal <= '1';
        wait for 1 ms;
        btn_signal <= '0';
        wait for 1 ms;
        btn_signal <= '1'; -- An actuall button press
        wait for 30 ms;
        btn_signal <= '0';
        wait for 1 ms;
        btn_signal <= '1';
        wait for 1 ms;
        btn_signal <= '0';
        wait for 1 ms;
        btn_signal <= '1';
        wait for 1 ms;
        btn_signal <= '0';
        wait for 130 ms; -- lets observe a few cycles of LED behaviour -> 25 Hz
        
        --Second 'normal' press
        btn_signal <= '1';
        wait for 20 ms;
        btn_signal <= '0';
        wait for 240 ms; -- lets observe a few cycles of LED behaviour -> 12.5 Hz
        
        --Third 'normal' press
        btn_signal <= '1';
        wait for 20 ms;
        btn_signal <= '0';
        wait for 480 ms; -- lets observe a few cycles of LED behaviour -> 6.25 Hz
        
        -- RESET
        RST <= '1';
        wait for 20 ns;
        btn_signal <= '0';
        wait for 40 ms; -- lets observe if everything has reseted  
        
     
        -- Stop the clock and hence terminate the simulation
        TbSimEnded <= '1';
        wait;
    end process;
    
    
end tb;

-- Configuration block below is required by some simulators. Usually no need to edit.

configuration cfg_tb_main of tb_main is
    for tb
    end for;
end cfg_tb_main;