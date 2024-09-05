library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.numeric_std.all;

entity main is
  Port (CLK : in std_logic; -- CLK source, Pin Y9 on zedBoard
        RST : in std_logic; -- RST button, Pin P16
        btn_signal : in std_logic; -- Button input signal, Pin R18

        LED : out std_logic_vector(5 downto 0) -- LED outputs, 5 used to display binary counter, 1 to blink
  );
end main;

architecture Behavioral of main is
    
    --Machine state signals 
    type state_t is (S0,S1,S2,S3,S4,S5);
    signal state: state_t := S0;
    signal next_state: state_t := S0;
    
    -- Signal to maintain output LED
    signal led_signal : std_logic_vector(5 downto 0) := "000000";
    
    -- Synchroniser's signals for external button
    signal sig_async_p1 : std_logic := '0';
    signal sig_sync : std_logic := '0';
    
    -- Edge detector signals
    signal previous_edge : std_logic_vector(3 downto 0);
    signal is_rising : boolean;
    signal is_falling : boolean;
    
    -- Debouncing signals
    signal debounce : std_logic_vector(3 downto 0) := "0000";
    
    -- Constants used to maintain frequency of LEDs
    constant c_CNT_50 : natural := 1000000; -- 50 Hz
    constant c_CNT_25 : natural := 2000000; -- 25 Hz
    constant c_CNT_12_5 : natural := 4000000; -- 12.5 Hz
    constant c_CNT_6_25 : natural := 8000000; -- 6.25 Hz
    constant c_CNT_3_12 : natural := 16000000; -- 3.125 Hz
    constant c_CNT_1_56 : natural := 32000000; -- 1.56 Hz

    constant c_CNT_2ms : natural := 200000; -- 2ms
    
    -- Counters
    signal r_CNT_50 : natural range 0 to c_CNT_50;
    signal r_CNT_25 : natural range 0 to c_CNT_25;
    signal r_CNT_12_5 : natural range 0 to c_CNT_12_5;
    signal r_CNT_6_25 : natural range 0 to c_CNT_6_25;
    signal r_CNT_3_12 : natural range 0 to c_CNT_3_12;
    signal r_CNT_1_56 : natural range 0 to c_CNT_1_56;
    
    signal r_CNT_2ms : natural range 0 to c_CNT_2ms;
    
    --Toggles
    signal r_TOGGLE_50 : std_logic := '1';
    signal r_TOGGLE_25 : std_logic := '1';
    signal r_TOGGLE_12_5 : std_logic := '1';
    signal r_TOGGLE_6_25 : std_logic := '1';
    signal r_TOGGLE_3_12 : std_logic := '1';
    signal r_TOGGLE_1_56 : std_logic := '1';
    
    --Counter used for binary counter
    signal counter : natural range 0 to 31;
begin

--Synchronization process for external button signal, waits at least 2CLK cycles to avoid metastability
SYNC_PROC : process(CLK) 
begin

    if(rising_edge(CLK)) then
    
        sig_async_p1 <= btn_signal;
        sig_sync <= sig_async_p1;
        
    end if;
        
end process;

--Debouncing using shift register
DEBOUNC_PROC : process(CLK, RST)
begin

    if RST = '1' then
    
        debounce <= "0000";
        r_CNT_2ms <= 0;

    elsif(rising_edge(CLK)) then
        
        if r_CNT_2ms = c_CNT_2ms then
            
            debounce(3) <= sig_sync;
            debounce(2) <= debounce(3);
            debounce(1) <= debounce(2);
            debounce(0) <= debounce(1);
            
            r_CNT_2ms <= 0;
            
        else
        
            r_CNT_2ms <= r_CNT_2ms + 1;
            
        end if;
        
        
    end if;
    
end process;

-- Edge detection based on debounced signal
EDGE_DETEC_PROC : process(CLK, RST) 
begin
    if RST = '1' then
    
        previous_edge <= "0000";
        is_falling <= false;
        is_rising <= false;
    
    elsif(rising_edge(CLK)) then
        
        previous_edge <= debounce;
        
        is_rising <= previous_edge = "1110" and debounce = "1111";
        is_falling <= previous_edge = "0001" and debounce = "0000";
    
    end if;
    
end process;

--Next state for state machine and update of a binary counter
STATE_CHANGER_PROC : process(CLK, RST)
begin
    if RST = '1' then
    
        next_state <= S0;
        counter <= 0;

    elsif rising_edge(CLK) then
        if is_rising then
                
            case state is
            
                when S0 =>
                    next_state <= S1;
                when S1 => 
                    next_state <= S2;
                when S2 => 
                    next_state <= S3;
                when S3 => 
                    next_state <= S4;
                when S4 => 
                    next_state <= S5;
                when S5 => 
                    next_state <= S0;
                    
                
            end case;
            
            counter <= counter + 1;
            
        end if;
    end if;
end process;

--Process maintaining diodes and state machine
DIODE_PROC : process(RST, CLK)
begin

    if RST = '1' then
    
        state <= S0;
        led_signal <= "000000";
    
    elsif rising_edge(CLK) then
    
        case state is
            when S0 =>
            
                if r_CNT_50 = c_CNT_50 - 1 then
                   
                    led_signal(0) <= r_TOGGLE_50;
                    r_TOGGLE_50 <= not r_TOGGLE_50;
                    r_cnt_50 <= 0;
                   
                else
                
                   r_cnt_50 <= r_cnt_50 + 1;
                   
                end if; 
                
                
                       
            when S1 =>
            
               if r_CNT_25 = c_CNT_25 - 1 then
               
                   led_signal(0) <= r_TOGGLE_25;
                   r_TOGGLE_25 <= not r_TOGGLE_25;
                   r_cnt_25 <= 0;
                    
               else
               
                   r_cnt_25 <= r_cnt_25 + 1;
                   
               end if; 
               
               

            when S2 =>
            
               if r_CNT_12_5 = c_CNT_12_5 - 1 then
                    
                   led_signal(0) <= r_TOGGLE_12_5;
                   r_TOGGLE_12_5 <= not r_TOGGLE_12_5;

                    r_cnt_12_5 <= 0;
                    
               else
               
                   r_cnt_12_5 <= r_cnt_12_5 + 1;
                   
               end if;
               
               

            when S3 =>
            
               if r_CNT_6_25 = c_CNT_6_25 - 1 then
               
                   led_signal(0) <= r_TOGGLE_6_25;
                   r_TOGGLE_6_25 <= not r_TOGGLE_6_25;
                   r_cnt_6_25 <= 0;
                   
               else
                   r_cnt_6_25 <= r_cnt_6_25 + 1;
               end if;
               
              

            when S4 =>
            
               if r_CNT_3_12 = c_CNT_3_12 - 1 then
                    
                   led_signal(0) <= r_TOGGLE_3_12;
                   r_TOGGLE_3_12 <= not r_TOGGLE_3_12;
                   r_cnt_3_12 <= 0;
                   
               else
               
                   r_cnt_3_12 <= r_cnt_3_12 + 1;
                   
               end if;
               
               
               
            when S5 =>
            
               if r_CNT_1_56 = c_CNT_1_56 - 1 then
                    
                   led_signal(0) <= r_TOGGLE_1_56;
                   r_TOGGLE_1_56 <= not r_TOGGLE_1_56;
                   r_cnt_1_56 <= 0;
                    
               else
               
                   r_cnt_1_56 <= r_cnt_1_56 + 1;
                   
               end if;
               
               
               
        end case;
        
        led_signal(5 downto 1) <= std_logic_vector(TO_UNSIGNED(counter,5)); 
        state <= next_state;
        
    end if;

end process;

-- Assign: led_signal(5 downto 1) -> binary counter  and   led_signal(0) -> blinking mode
LED <= led_signal;


end Behavioral;
