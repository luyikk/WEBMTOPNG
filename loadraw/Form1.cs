using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;

namespace loadraw
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void Button1_Click(object sender, EventArgs e)
        {
            using (var stream = new FileStream("a.raw", FileMode.Open, FileAccess.Read))
            {
                var image = new Bitmap(1920, 1080, System.Drawing.Imaging.PixelFormat.Format32bppArgb);

                var read = new BinaryReader(stream);

                for (int y = 0; y < 1080; y++)
                {
                    for (int x = 0; x < 1920; x++)
                    {
                        
                        var b = read.ReadByte();
                        var g = read.ReadByte();
                        var r = read.ReadByte();
                        var a = read.ReadByte();

                        image.SetPixel(x, y, Color.FromArgb(a, r, g, b));

                    }
                }

                this.pictureBox1.Image = image;

            }
        }
    }
}
