﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace BaluEditor
{
    public partial class MainWindow : Form
    {
        public MainWindow()
        {
            InitializeComponent();
            baluEditorControl1.InitializeEngine();

            InitializeCreateNodeContextMenu();
        }

        void InitializeCreateNodeContextMenu()
        {
            var n = WorldContextMenu.Items.Add("Create") as ToolStripMenuItem;

            var i = n.DropDown.Items.Add("Material");
            i.Tag = new Editor.TWolrdTreeNodeTag(Editor.TNodeType.Material);
            i.Click += (object sender, EventArgs e) => { baluEditorControl1.CreateMaterial(); };

            i = n.DropDown.Items.Add("Sprite");
            i.Tag = new Editor.TWolrdTreeNodeTag(Editor.TNodeType.Sprite);
            i.Click += (object sender, EventArgs e) => { baluEditorControl1.CreateSprite(); };

            i = n.DropDown.Items.Add("PhysBody");
            i.Tag = new Editor.TWolrdTreeNodeTag(Editor.TNodeType.PhysBody);
            i.Click += (object sender, EventArgs e) => { baluEditorControl1.CreatePhysBody(); };

            i = n.DropDown.Items.Add("Class");
            i.Tag = new Editor.TWolrdTreeNodeTag(Editor.TNodeType.Class);
            i.Click += (object sender, EventArgs e) => { baluEditorControl1.CreateClass(); };

            i = n.DropDown.Items.Add("Scene");
            i.Tag = new Editor.TWolrdTreeNodeTag(Editor.TNodeType.Scene);
            i.Click += (object sender, EventArgs e) => { baluEditorControl1.CreateScene(); };
        }

        void i_Click(object sender, EventArgs e)
        {
            throw new NotImplementedException();
        }

        bool times_is_work = false;
        private void timer1_Tick(object sender, EventArgs e)
        {
            if (times_is_work) return;
            times_is_work = true;
            baluEditorControl1.Render();
            times_is_work = false;
        }

        private void treeView1_NodeMouseClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            var n = e.Node.Tag as Editor.TWolrdTreeNodeTag;
            if(n!=null)
            {
                if(!n.IsSpecialNode)
                {
                    baluEditorControl1.SetSelectedWorldNode(n);
                }
            }
        }

        private void baluEditorControl1_MouseEnter(object sender, EventArgs e)
        {
            baluEditorControl1.Focus();
        }

        private void treeView1_NodeMouseDoubleClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            var n = e.Node.Tag as Editor.TWolrdTreeNodeTag;
            if (n != null)
            {
                if (!n.IsSpecialNode)
                {
                    baluEditorControl1.SetEditedWorldNode(n);
                }
            }
        }
    }
}
