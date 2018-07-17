//using UnityEngine;
using System.Collections;
using System;
using System.Runtime.InteropServices;


namespace Mri
{


    public struct SendMDA
    {
        public int mcw;
        public float a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s;
        public int dummy1, dummy2;
    };

    

    public class DllMotionInterface : IDisposable
    {


        public bool Connected { get; private set; }

        //private Transform subjectCarTransform;

        //private float forwardVelocity;
        //private float forwardAcceleration;


        //private Vector3 velocity;
        //private Vector3 prevVelocity1;
        //private Vector3 prevVelocity2;

        //private Vector3 angularVelocity;
        //private Vector3 prevAngularVelocity1;
        //private Vector3 prevAngularVelocity2;

        //private float prevDeltaTime1;
        //private float prevDeltaTime2;
        //private Vector3 acceleration;
        //private Vector3 angularAcceleration;

        //private Rigidbody vehicleRigidbody;

        //private SendMDA mdaData;


        [DllImport("MotionLib", CallingConvention = CallingConvention.Cdecl)]
        public extern static void MotionBaseInit(string h);

        [DllImport("MotionLib", CallingConvention = CallingConvention.Cdecl)]
        public extern static void UpdateMotionBase();

        [DllImport("MotionLib", CallingConvention = CallingConvention.Cdecl)]
        public extern static void SetSmoothing(bool w);

        [DllImport("MotionLib", CallingConvention = CallingConvention.Cdecl)]
        public extern static int MotionStart(string host);



        [DllImport("MotionLib", CallingConvention = CallingConvention.Cdecl)]
        public extern static void ShutDownMotionBase();

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

        [DllImport("MotionLib", CallingConvention = CallingConvention.Cdecl)]
        private extern static void SetMDA(SendMDA mda_rev);

       



        public DllMotionInterface()
        {
            string MBIP = "128.127.55.120";
            if (MotionStart(MBIP) == 1) {
                Connected = true;
            }
            else
            {
                Connected = false;
            }
            
        }

        //public void Dispose()
        //{
        //    throw new NotImplementedException();
        //}

        public void sendMDA(SendMDA mda)
        {
            SetMDA(mda);
        }

        public bool Disconnect()
        {
            try
            {
                ShutDownMotionBase();
                this.Connected = false;
            }
            catch (Exception)
            {

                throw new Exception("Disconnect Motion Platform has failed.");
            }
            return this.Connected;
        }



        #region IDisposable Support
        private bool disposedValue = false; // To detect redundant calls

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (this.Connected)
                {
                    // TODO: dispose managed state (managed objects).
                    Disconnect();
                }

                // TODO: free unmanaged resources (unmanaged objects) and override a finalizer below.
                // TODO: set large fields to null.

                disposedValue = true;
            }
        }

        // TODO: override a finalizer only if Dispose(bool disposing) above has code to free unmanaged resources.
        ~DllMotionInterface()
        {
            // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
            Dispose(false); 
        }

        // This code added to correctly implement the disposable pattern.
        public void Dispose()
        {
            // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
            Dispose(true);
            // TODO: uncomment the following line if the finalizer is overridden above.
            GC.SuppressFinalize(this);
        }
        #endregion
    }
}


