using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO.Ports;
using System.Windows.Forms;
using System.Net.Configuration;
using System.IO;
using System.Threading;
using System.Diagnostics;
using System.Drawing.Text;
using System.Globalization;
using System.Windows.Forms.VisualStyles;
using SimpleTCP;
using CsvHelper;

namespace DataGrapher
{
    public partial class SleeveResistance : Form
    {
        public int Counter = 0;  

        //private members

        //serial port stuff
        private string portName = "COM1";
        private SerialPort serialPort;
        private int BAUDRATE = 1000000;
        private int READ_TIMEOUT = 300;


        //message reading stuff
        private List<byte> message = new List<byte>();
        private List<byte> dummy_message = new List<byte>();
        private bool found_starting_point = false;
        private int msg_counter = 0;
        private bool collect = false;
        string[] empty = new string[50];

        //file i/o
        private string log_data_filepath = "";
        private string scenario_filepath = "";
        DataTable scenario_data_table = new DataTable();
        private bool written = false;
        private bool file_specified = false;
        



        //scenario population
        private string current_scenario_string = "None";
        private string next_scenario_start_time = "00:00:00";
        private int scenario_index = 0;
        
        


        //timers
        private DateTime current_time = DateTime.Now;
        private DateTime msg_rcv = DateTime.Now;
        private DateTime next_start_time = DateTime.Now;

        //controls
        private List<Control> pinTexts = new List<Control>();





        SimpleTcpClient client;


        public class calculatedhand
        {
            public double Thumbs { get; set; }
            public double Point { get; set; }
            public double Middle { get; set; }
            public double Ring { get; set; }
            public double Pinky { get; set; }
        }




        public SleeveResistance()
        {
            InitializeComponent();

            
            enumerateCOMPorts();    //enumerate COMPorts and update COM Port combo box


 /*           pinTexts = GetAll(this, typeof(TextBox));
            foreach(TextBox t in pinTexts)
            {
                //Console.WriteLine(t.Name);
            }*/
            


        }

/*        private List<Control> GetAll(Control control, Type type)
        {
            var controls = control.Controls.Cast<Control>();

            var control_list = controls.SelectMany(ctrl => GetAll(ctrl, type))
                                      .Concat(controls)
                                      .Where(c => c.GetType() == type)
                                      .Where(c => c.Name.Contains("_value"));

            return control_list.ToList();
                                     
        }*/


        private void enumerateCOMPorts()
        {
            string[] ports = SerialPort.GetPortNames();
            com_ports.DataSource = ports;
        }

        //handle serial events here
        private void serialDataHandler(object sender, SerialDataReceivedEventArgs e)
        {
            msg_rcv = DateTime.Now;
            if (found_starting_point && collect && file_specified && statusStrip1.BackColor != Color.Green)
            {
                statusStrip1.BackColor = Color.Green;
                timer1.Enabled = true;
            }
            SerialPort sData = sender as SerialPort;

            string[] data;
            string[] ordered_data;

            try
            {
                alignAndRead(sData, 92);

                //Console.WriteLine(message[91]);

                if((message.Count == 92) && (message[91] == 10))
                {
                    data = decodeData(message.ToArray());
                    if(data != null)
                    {
                        ordered_data = orderData(data);
                    }
                    else
                    {
                        found_starting_point = false;
                        ordered_data = empty;
                    }
                    
                    message.Clear();
                }
                else
                {
                    found_starting_point = false;
                    this.Invoke((MethodInvoker)delegate { consoleBox.AppendText("Corrupt data recieved\r\n"); });
                    ordered_data = empty;
                    message.Clear();
                }


                if (program_timer.Enabled && found_starting_point && collect && file_specified)
                {
                    string data_line = current_scenario_string + "," + msg_rcv.ToString("HH:mm:ss.fff") + "," + String.Join(",", ordered_data);
                    StreamWriterWrapper.Instance.WriteLine(log_data_filepath, data_line);

                    this.Invoke((MethodInvoker)delegate { updatePinValues(ordered_data); });
                 

                    if (msg_counter % 10 == 0)
                    {
                        this.Invoke((MethodInvoker)delegate { consoleBox.AppendText("New Data Added to Scenario: " + current_scenario_string + "\r\n"); });
                        msg_counter = 0;
                    }

                    msg_counter++;
                }else if (!file_specified)
                {
                    if (msg_counter % 5 == 0)
                    {
                        this.Invoke((MethodInvoker)delegate { consoleBox.AppendText("Data Incoming, but no log file specified...please specify a log file now\r\n"); });
                        msg_counter = 0;
                    }
                    msg_counter++;
                    this.Invoke((MethodInvoker)delegate { updatePinValues(ordered_data); });
                }
                else
                {
                    //if (msg_counter % 5 == 0)
                    //{
                    //    this.Invoke((MethodInvoker)delegate { consoleBox.AppendText("Data Incoming\r\n"); });
                    //    msg_counter = 0;
                    //}
                    //msg_counter++;

                    this.Invoke((MethodInvoker)delegate { updatePinValues(ordered_data); });
                }



            }catch(Exception ex)
            {
                Console.WriteLine(ex.Message + ':' + ex.StackTrace);
            }


        }

        private string[] orderData(string[] data)
        {

            string[] ordered_data = new string[data.Length];

            ordered_data[0] = data[3];
            ordered_data[1] = data[7];
            ordered_data[2] = data[11];
            ordered_data[3] = data[15];
            ordered_data[4] = data[31];
            ordered_data[5] = data[27];

            ordered_data[6] = data[23];
            ordered_data[7] = data[19];
            ordered_data[8] = data[2];
            ordered_data[9] = data[6];
            ordered_data[10] = data[10];
            ordered_data[11] = data[14];
            ordered_data[12] = data[30];

            ordered_data[13] = data[26];
            ordered_data[14] = data[22];
            ordered_data[15] = data[18];
            ordered_data[16] = data[16];
            ordered_data[17] = data[20];
            ordered_data[18] = data[24];

            ordered_data[19] = data[28];
            ordered_data[20] = data[12];
            ordered_data[21] = data[8];
            ordered_data[22] = data[4];
            ordered_data[23] = data[0];
            ordered_data[24] = data[17];
            ordered_data[25] = data[21];

            ordered_data[26] = data[25];
            ordered_data[27] = data[29];
            ordered_data[28] = data[13];
            ordered_data[29] = data[9];
            ordered_data[30] = data[5];
            ordered_data[31] = data[1];

            ordered_data[32] = data[32];

            ordered_data[33] = data[33];
            ordered_data[34] = data[34];
            ordered_data[35] = data[35];
            ordered_data[36] = data[36];
            ordered_data[37] = data[37];
            ordered_data[38] = data[38];
            ordered_data[39] = data[39];
            ordered_data[40] = data[40];
            ordered_data[41] = data[41];
            ordered_data[42] = data[42];
            ordered_data[43] = data[43];


            return ordered_data;
        }


        private void alignAndRead(SerialPort sData, int msg_length)
        {
            //int num_bytes = sData.BytesToRead;
            byte[] message_bytes = new byte[1];
            byte current_byte = 0xFF;
            int endPacketCounter = 0;
            int indexOfFirstPacketEnd = 0;
            int indexOfSecondPacketEnd = 0;
            //Console.WriteLine("Incoming Bytes: {0}", num_bytes);
            while (!found_starting_point)
            {
                sData.Read(message_bytes, 0, 1);
                current_byte = message_bytes[0];
                //Console.WriteLine("current byte: {0}, end packet counter: {1}, index1: {2}, index2: {3}", current_byte, endPacketCounter, indexOfFirstPacketEnd, indexOfSecondPacketEnd); ;
                dummy_message.Add(current_byte);
                if (endPacketCounter == 1 && current_byte == 10)
                {
                    if (dummy_message[dummy_message.Count - 2] == 13)
                    {
                        indexOfSecondPacketEnd = dummy_message.Count - 2;
                        if ((indexOfSecondPacketEnd - indexOfFirstPacketEnd) == msg_length)
                        {
                            //Console.WriteLine("found window");
                            found_starting_point = true;
                            endPacketCounter = 2;
                            this.Invoke((MethodInvoker)delegate { consoleBox.AppendText("Found the align pack \r\n"); });
                            dummy_message.Clear();
                            break;
                        }
                        else
                        {
                            indexOfFirstPacketEnd = dummy_message.Count - 2;
                            indexOfSecondPacketEnd = 0;

                        }
                    }
                }
                else if (current_byte == 10)
                {
                    this.Invoke((MethodInvoker)delegate { consoleBox.AppendText("Align with 10 \r\n"); });

                    if (dummy_message[dummy_message.Count - 2] == 13)
                    {
                        endPacketCounter = 1;
                        indexOfFirstPacketEnd = dummy_message.Count - 2;
                    }
                }
            }

            while (message.Count < msg_length)
            {
                sData.Read(message_bytes, 0, 1);
                current_byte = message_bytes[0];
                message.Add(current_byte);
            }
        }


        private string[] decodeData(byte[] message)
        {


            List<string> decoded = new List<string>();



            byte[] resistances = new byte[64];
            Array.Copy(message, 0, resistances, 0, resistances.Length);//substring with resistances bytes

            byte[] ecg = new byte[4];
            Array.Copy(message, 64, ecg, 0, ecg.Length); //substring with ecg value

            byte[] bno = new byte[22];
            Array.Copy(message, 68, bno, 0, bno.Length); //substring with bno values



            byte lsb;
            byte lsb8;
            byte msb16;
            byte msb24;

            ushort num_combined_2byte;
            uint num_combined_4byte;


            //decode resistances
            int j = 0;
            for (int i = 0; i < resistances.Length / 2; i++)
            {
                j = i * 2;
                lsb = resistances[j];

                lsb8 = resistances[j + 1];

                num_combined_2byte = (ushort)((lsb8 << 8) | (lsb));

                decoded.Add(num_combined_2byte.ToString());

            }

            //decode ecg
            lsb = ecg[0];
            lsb8 = ecg[1];
            msb16 = ecg[2];
            msb24 = ecg[3];

            num_combined_4byte = (uint)(msb24 << 24 | ((msb16 << 16) | ((lsb8 << 8) | (lsb))));
            //Console.WriteLine(num_combined_4byte);
            decoded.Add(num_combined_4byte.ToString());

            //decode bno
            j = 0;
            for (int i = 0; i < bno.Length / 2; i++)
            {
                j = i * 2;
                lsb = bno[j];
                lsb8 = bno[j + 1];


                num_combined_2byte = (ushort)((lsb8 << 8) | (lsb));


                decoded.Add(num_combined_2byte.ToString());
            }


            return decoded.ToArray();
        }

        private void updatePinValues(string[] data)
        {

            //TODO: I guarantee there is a better way to do this, use LINQ or something to get texboxes

                
                P0_value.Text = data[0];
                P1_value.Text = data[1];
                P2_value.Text = data[2];
                P3_value.Text = data[3];
                P4_value.Text = data[4];
                P5_value.Text = data[5];
                P6_value.Text = data[6];
                P7_value.Text = data[7];
                P8_value.Text = data[8];
                P9_value.Text = data[9];
                P10_value.Text = data[10];
                P11_value.Text = data[11];
                P12_value.Text = data[12];
                P13_value.Text = data[13];
                P14_value.Text = data[14];
                P15_value.Text = data[15];
                P16_value.Text = data[16];
                P17_value.Text = data[17];
                P18_value.Text = data[18];
                P19_value.Text = data[19];
                P20_value.Text = data[20];
                P21_value.Text = data[21];
                P22_value.Text = data[22];
                P23_value.Text = data[23];
                P24_value.Text = data[24];
                P25_value.Text = data[25];
                P26_value.Text = data[26];
                P27_value.Text = data[27];
                P28_value.Text = data[28];
                P29_value.Text = data[29];
                P30_value.Text = data[30];
                P31_value.Text = data[31];
            




        }


        private void tableLayoutPanel1_Paint(object sender, PaintEventArgs e)
        {

        }


        private void tableLayoutPanel4_Paint(object sender, PaintEventArgs e)
        {

        }

        private void connect_disconnect_button_Click(object sender, EventArgs e)
        {
            if(serialPort == null)
            {
                serialPort = new SerialPort(portName, BAUDRATE, Parity.None, 8, StopBits.One);
                serialPort.DataReceived += new SerialDataReceivedEventHandler(serialDataHandler);
                serialPort.ReadTimeout = READ_TIMEOUT;
            }


            if (!serialPort.IsOpen)
            {

                bool port_connection_success = false;

                while (!port_connection_success)
                {
                    
                    serialPort.Open();
                    serialPort.DiscardInBuffer();
                    this.Invoke((MethodInvoker)delegate { consoleBox.AppendText("Opening serial port...\r\n"); });

                    if (serialPort.IsOpen) port_connection_success = true;
                    else this.Invoke((MethodInvoker)delegate { consoleBox.AppendText("Cannot open port, trying again..."); });

                }

                this.Invoke((MethodInvoker)delegate { consoleBox.AppendText("Opened\r\n"); });
            }
            else
            {
                serialPort.Close();
                this.Invoke((MethodInvoker)delegate { consoleBox.AppendText("Closed serial port\r\n"); });
                updatePinValues(empty);
                serialPort.Dispose();
            }

        }

        private void com_ports_SelectedIndexChanged(object sender, EventArgs e)
        {
            portName = com_ports.SelectedItem.ToString();
        }

        private void csv_file_path_TextChanged(object sender, KeyEventArgs e)
        {
            if(e.KeyCode == Keys.Enter)
            {
                if (csv_file_path.Text != "")
                {
                    log_data_filepath = csv_file_path.Text + ".csv";
                    log_data_filepath = Path.GetFullPath(log_data_filepath);
                    consoleBox.AppendText("Log File: " + log_data_filepath + "\n");
                    file_specified = true;

                    if (!File.Exists(log_data_filepath))
                    {
                        //TODO{update this to also include accelerations?}
                        StreamWriterWrapper.Instance.WriteLine(log_data_filepath, "Scenario,Time,P0,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11,P12,P13,P14,P15,P16,P17,P18,P19,P20,P21,P22,P23,P24,P25,P26,P27,P28,P29,P30,P31,ECG,AX,AY,AZ,QW,QX,QY,QZ,CS,CA,CG,CM");
                        //sw.WriteLine("W,X,Y,Z");
                    }
                }
                else
                {
                    consoleBox.AppendText("Log File Path Cannot be Empty");
                }
            }
        }



        private void SleeveResistance_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (!written)
            {
                StreamWriterWrapper.Instance.Dispose();
            }
            ;
        }

        private void SleeveResistance_Load(object sender, EventArgs e)
        {

        }

        private void csv_submit_Click(object sender, EventArgs e)
        {
            if(csv_file_path.Text != "")
            {
                log_data_filepath = csv_file_path.Text + ".csv";
                log_data_filepath = Path.GetFullPath(log_data_filepath);
                consoleBox.AppendText("Log File: " + log_data_filepath + "\n");
                file_specified = true;

                if (!File.Exists(log_data_filepath))
                {
                    //TODO{update this to also include accelerations?}
                    StreamWriterWrapper.Instance.WriteLine(log_data_filepath, "Scenario,Time,P0,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11,P12,P13,P14,P15,P16,P17,P18,P19,P20,P21,P22,P23,P24,P25,P26,P27,P28,P29,P30,P31,ECG,AX,AY,AZ,QW,QX,QY,QZ,CS,CA,CG,CM");
                    //sw.WriteLine("W,X,Y,Z");
                }
            }
            else
            {
                consoleBox.AppendText("Log File Path Cannot be Empty");
            }



        }



        private void program_timer_Tick(object sender, EventArgs e)
        {
            current_time = DateTime.Now;
            computer_time.Text = current_time.ToString("HH:mm:ss");

            if(DateTime.Compare(current_time,next_start_time) > 0)
            {
                collect = true;
                
                if(scenario_index < scenario_data_table.Rows.Count && (scenario_data_table.Rows[scenario_index][0].ToString() != "Stop" || scenario_data_table.Rows[scenario_index][0].ToString() != "stop"))
                {
                    current_scenario.Text = scenario_data_table.Rows[scenario_index][0].ToString();
                    current_scenario_string = current_scenario.Text;
                    if (scenario_index < scenario_data_table.Rows.Count - 1)
                    {
                        next_scenario.Text = scenario_data_table.Rows[scenario_index + 1][0].ToString();
                        next_scenario_start_time = scenario_data_table.Rows[scenario_index+1][1].ToString();
                        next_scenario_countdown.Text = scenario_data_table.Rows[scenario_index][1].ToString();
                        current_scenario_timer.Text = scenario_data_table.Rows[scenario_index + 1][1].ToString();
                    }
                    else if (scenario_data_table.Rows[scenario_index][0].ToString() == "Stop" || scenario_data_table.Rows[scenario_index][0].ToString() == "stop")
                    {
                        program_timer.Stop();
                        collect = false;
                        StreamWriterWrapper.Instance.Dispose();
                        written = true;
                        this.Invoke((MethodInvoker)delegate { consoleBox.AppendText("Collection Finished"); });
                        statusStrip1.BackColor = Color.Red;
                        timer1.Enabled = false;
                    }

                    scenario_index++;
                    next_start_time = DateTime.ParseExact(next_scenario_start_time, "HH:mm:ss", CultureInfo.GetCultureInfo("en-US"));
                }


                
            }
            
            

            



        }

        private void start_button_Click(object sender, EventArgs e)
        {
            try
            {
                
                scenario_index = 0;
                current_scenario.Text = current_scenario_string;
                next_scenario.Text = scenario_data_table.Rows[scenario_index][0].ToString();
                next_scenario_start_time = scenario_data_table.Rows[scenario_index][1].ToString();
                current_scenario_timer.Text = next_scenario_start_time;
                program_timer.Start();
            }
            catch(IndexOutOfRangeException ex)
            {
                consoleBox.AppendText("Problem loading scenario list...please make sure you have specified a scenario configuration file\r\n");
            }

  

            next_start_time = DateTime.ParseExact(next_scenario_start_time, "HH:mm:ss", CultureInfo.GetCultureInfo("en-US"));



        }






        private void scenario_file_chooser_Click(object sender, EventArgs e)
        {
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {

                scenario_filepath = openFileDialog1.FileName;
                chosen_scenario_file.Text = scenario_filepath;

            }

            scenario_data_table = new DataTable();
            scenario_data_table.Columns.Add("Scenario Name:");
            scenario_data_table.Columns.Add("Start Time:");
            string filePath = scenario_filepath;
            StreamReader streamReader = new StreamReader(filePath);
            string[] totalData = new string[File.ReadAllLines(filePath).Length];
            while (!streamReader.EndOfStream)
            {
                totalData = streamReader.ReadLine().Split(',');
                scenario_data_table.Rows.Add(totalData[0], totalData[1]);
            }
            dataGridView1.DataSource = scenario_data_table;
        }

        private void tableLayoutPanel5_Paint(object sender, PaintEventArgs e)
        {

        }

        private void stop_button_Click(object sender, EventArgs e)
        {

            program_timer.Stop();
            if(!written) StreamWriterWrapper.Instance.Dispose();
            written = true;
        }

        private void statusStrip1_ItemClicked(object sender, ToolStripItemClickedEventArgs e)
        {

        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            if (Counter==0)
            {
                Counter = 1;
                client = new SimpleTcpClient();
                client.Delimiter = 0x13;//enter
                client.StringEncoder = Encoding.UTF8;
                //client.DataReceived += Client_DataReceived;
                client.Connect("127.0.0.1", Convert.ToInt32("3000"));
            }
            else
            {
                Counter++;
                //client.WriteLine("p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p18,p19,p20,p21,p22,p23,p24,p25,p26,p27,p28,p29,frame");//, TimeSpan.FromSeconds(1));
                var reader = new StreamReader(@"C:\Users\Arvin\Dropbox\2019-2020\projects\Shared\ECE\Prof. Servati\github\Datafeed\Datafeed1\V1\UnityDatafeedV1.0\Unity\Assets\exp100demo.csv");
                var csv = new CsvReader(reader, System.Globalization.CultureInfo.CurrentCulture);
                csv.Configuration.HasHeaderRecord = false;
                var record = csv.GetRecords<calculatedhand>();
                List<calculatedhand> records = record.ToList();
                const int number_of_data = 1148;

                if (Counter < number_of_data)
                {
                    client.WriteLine(records[Counter].Thumbs.ToString()+","+ records[Counter].Point.ToString()+","+ records[Counter].Middle.ToString()+","+ records[Counter].Ring.ToString()+","+ records[Counter].Pinky.ToString()+", frame:\n");
                }
                else
                {
                    timer1.Enabled = false;
                }
            }

        }

        private void sql_submit_Click(object sender, EventArgs e)
        {
            timer1.Enabled = true;
        }
    }
}
