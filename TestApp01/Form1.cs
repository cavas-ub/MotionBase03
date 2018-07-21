using Mri;
using System;
using System.Windows.Forms;

namespace TestApp01
{
    public partial class Form1 : Form
    {
        //public struct SendMDA
        //{
        //    public int mcw;
        //    public float a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s;
        //    public int dummy1, dummy2;
        //};

        ////[DllImport("MotionDll02", CallingConvention = CallingConvention.Cdecl)]
        ////public extern static void MotionBaseInit(string h);

        //[DllImport("MotionDll02", CallingConvention = CallingConvention.Cdecl)]
        //public extern static void UpdateMotionBase();

        //[DllImport("MotionDll02", CallingConvention = CallingConvention.Cdecl)]
        //public extern static void SetSmoothing(bool w);

        //[DllImport("MotionDll02", CallingConvention = CallingConvention.Cdecl)]
        //public extern static int MotionStart(string host);

        //[DllImport("MotionDll02", CallingConvention = CallingConvention.Cdecl)]
        //public extern static void ShutDownMotionBase();

        //[DllImport("MotionDll02", CallingConvention = CallingConvention.Cdecl)]
        //public extern static void SetRoll(float f);

        //[DllImport("MotionDll02", CallingConvention = CallingConvention.Cdecl)]
        //public extern static void SetPitch(float f);

        //[DllImport("MotionDll02", CallingConvention = CallingConvention.Cdecl)]
        //public extern static void SetYaw(float f);

        //[DllImport("MotionDll02", CallingConvention = CallingConvention.Cdecl)]
        //public extern static void SetHeave(float f);

        //[DllImport("MotionDll02", CallingConvention = CallingConvention.Cdecl)]
        //public extern static void SetSurge(float f);

        //[DllImport("MotionDll02", CallingConvention = CallingConvention.Cdecl)]
        //public extern static void SetSway(float f);


        //[DllImport("MotionDll02", CallingConvention = CallingConvention.Cdecl)]
        //public extern static void SetMDA(SendMDA mda_rev);

        public float value_to_send;
        SendMDA sm;

        private DllMotionInterface dllMotionInterface;

        public Form1()
        {
            InitializeComponent();
            trackBar1.Value = trackBar1.Maximum / 2;
            sm = new SendMDA();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            sm.o = 0;
            

            // initializeMB();
            InitMotionControl();


        }


        //private void initializeMB()
        //{
        //    string MBIP = "128.127.55.120";
        //    //MotionBaseInit(MBIP);
        //    MotionStart(MBIP);

        //    //SetSmoothing(true);
        //    //SetHeave(0);
        //    //SetPitch(0);
        //    //SetSurge(0);
        //    //SetRoll(0);
        //    //SetYaw(0);
        //    //SetSway(0);
        //    //UpdateMotionBase();

        //    //Thread.Sleep(8000); //wait 8 sec
        //    //button1.Enabled = false;
        //    button2.Enabled = true;
        //}

        private void button2_Click(object sender, EventArgs e)
        {
            //ShutDownMotionBase();
            //button1.Enabled = true;
            //button2.Enabled = false;

            dllMotionInterface.Disconnect();
            dllMotionInterface = null;
            Console.Beep();
            Console.Beep();

        }

        private void trackBar1_Scroll(object sender, EventArgs e)
        {
            //labelSliderValue.Text = trackBar1.Value.ToString();
            const float MIN_VALUE = -0.2f;
            const float MAX_VALUE = 0.2f;

            float range = MAX_VALUE - MIN_VALUE;
            float value_slider = ((float)trackBar1.Value / trackBar1.Maximum);  // range 0 - 1
            float value = MIN_VALUE + value_slider * range;
            value_to_send = value;
            labelSliderValue.Text = value.ToString();
        }

        private void timer1_Tick(object sender, EventArgs e)
        {

            label1.Text = "Value to send=" + value_to_send;

            //SetPitch(value_to_send);
            //UpdateMotionBase();
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void button3_Click(object sender, EventArgs e)
        {
            //MessageBox.Show("Value to send=" + value_to_send);
            timer1.Enabled = true;
            
            //SetPitch(0);
        }

        private void button4_Click(object sender, EventArgs e)
        {
            timer1.Enabled = false;
            label1.Text = "STOPPED";
        }

        private void button5_Click(object sender, EventArgs e)
        {
            
            //MAX_HEAVE -.4572

            sm.o -= 0.001f;

            if (sm.o < -0.4f)
                sm.o = -0.4f;

            //SetMDA(sm);

            dllMotionInterface.sendMDA(sm);

            
            lblValueZ.Text = sm.o.ToString();
        }

        private void button6_Click(object sender, EventArgs e)
        {
            sm.o += 0.001f;

            if (sm.o > 0.4f)
                sm.o = 0.4f;

            //SetMDA(sm);
            dllMotionInterface.sendMDA(sm);

            lblValueZ.Text = sm.o.ToString();
        }







        private void InitMotionControl()
        {
            try
            {
                dllMotionInterface = new DllMotionInterface();
                Console.Beep();

            }
            catch (System.Exception err)
            {
                Console.WriteLine("Error: " + err.Message.ToString());
                //Debug.LogError(err.Message);
            }
        }
    }
}
