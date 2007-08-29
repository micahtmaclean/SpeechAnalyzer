using System;
using System.Reflection;
using System.Resources;
using System.Xml;
using System.Data.OleDb;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;
using System.Diagnostics;

namespace SIL.SpeechTools.Utils
{
	/// ----------------------------------------------------------------------------------------
	/// <summary>
	/// Read and Write settings file
	/// </summary>
	/// ----------------------------------------------------------------------------------------
	public class SettingsHandler
	{
		protected const string kRootNodeName = "settings";
		protected const string kMiscSettingsNode = kRootNodeName + "/misc";
		private const string kLastPrjNode = kMiscSettingsNode + "/lastproject";
		private const string kGridsNode = kRootNodeName + "/grids";
		private const string kWindowStatesNode = kRootNodeName + "/windowstates";

		protected string m_settingsFile;
		protected XmlDocument m_xmlDoc;

		/// --------------------------------------------------------------------------------
		/// <summary>
		/// SettingsHandler class constructor
		/// </summary>
		/// <param name="settingsFile"></param>
		/// --------------------------------------------------------------------------------
		public SettingsHandler(string settingsFile)
		{
			if (settingsFile == null)
				return;

			m_settingsFile = settingsFile;
			m_xmlDoc = new XmlDocument();
			
			try
			{
				m_xmlDoc.Load(m_settingsFile);
			}
			catch
			{
				CreateNewSettingsFile();
			}
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// 
		/// </summary>
		/// ------------------------------------------------------------------------------------
		protected virtual void CreateNewSettingsFile()
		{
			XmlWriterSettings settings = new XmlWriterSettings();
			settings.Indent = true;
			settings.IndentChars = "\t";
			settings.CloseOutput = true;
			XmlWriter writer = XmlWriter.Create(m_settingsFile, settings);
			writer.WriteStartElement(kRootNodeName);
			writer.WriteEndElement();
			writer.Flush();
			writer.Close();
			m_xmlDoc.Load(m_settingsFile);
		}

		#region Properties
		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Gets or sets the last database loaded.
		/// </summary>
		/// ------------------------------------------------------------------------------------
		public string LastProject
		{
			set {SetStringValue(kLastPrjNode, "path", value);}
			get
			{
				if (m_xmlDoc == null)
					return null;

				XmlNode node = m_xmlDoc.SelectSingleNode(kLastPrjNode);
				if (node == null)
					return null;

				return XMLHelper.GetAttributeValue(node, "path");
			}
		}
		
		#endregion

		#region Form Related Methods
		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Saves window properties from a form to an XmlDocument.
		/// </summary>
		/// <param name="frm">Form to save settings from</param>
		/// <returns>True on success</returns>
		/// ------------------------------------------------------------------------------------
		public void SaveFormProperties(Form frm)
		{
			if (!frm.Visible)
				return;

			if (m_xmlDoc == null)
				CreateNewSettingsFile();

			// Don't bother saving a form's minimized state.
			if (frm.WindowState == FormWindowState.Minimized)
				return;

			XmlNode node = VerifyNodeExists(kWindowStatesNode);
			if (node == null)
				return;

			RemoveChildNode(node, frm.Name);

			XmlElement element = m_xmlDoc.CreateElement("window");
			element.SetAttribute("id", frm.Name);

			// Save the window size and location when it's state is normal.
			if (frm.WindowState == FormWindowState.Maximized)
				element.SetAttribute("state", "Maximized");
			else if (frm.WindowState == FormWindowState.Normal)
			{
				element.SetAttribute("state", "Normal");
				element.SetAttribute("left", frm.Left.ToString());
				element.SetAttribute("top", frm.Top.ToString());
				element.SetAttribute("width", frm.Width.ToString());
				element.SetAttribute("height", frm.Height.ToString());
			}

			node.AppendChild(element);
			m_xmlDoc.Save(m_settingsFile);
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Assigns window properties to a form from an XmlDocument.
		/// </summary>
		/// <param name="frm">Form to load settings into</param>
		/// <returns>True on success</returns>
		/// ------------------------------------------------------------------------------------
		public bool LoadFormProperties(Form frm)
		{
			if (m_xmlDoc == null || frm == null)
				return false;

			XmlNode node = FindChildNode(kWindowStatesNode, frm.Name);

			if (node == null) 
				return false;

			frm.Height = XMLHelper.GetIntFromAttribute(node, "height", frm.Height);
			frm.Width = XMLHelper.GetIntFromAttribute(node, "width", frm.Width);
			frm.Top = XMLHelper.GetIntFromAttribute(node, "top", frm.Top);
			frm.Left = XMLHelper.GetIntFromAttribute(node, "left", frm.Left);

			frm.WindowState = (XMLHelper.GetAttributeValue(node, "state") == "Maximized" ?
				FormWindowState.Maximized :	frm.WindowState = FormWindowState.Normal);

			return true;
		}

		#endregion

		#region Grid Related Methods
		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Saves properties of the specified grid.
		/// </summary>
		/// <param name="grid">Grid for which settings are being saved</param>
		/// <returns>True on success</returns>
		/// ------------------------------------------------------------------------------------
		public void SaveGridProperties(DataGridView grid)
		{
			SaveGridProperties(grid, null);
		}
		
		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Saves properties of the specified grid.
		/// </summary>
		/// <param name="grid">Grid for which settings are being saved</param>
		/// <returns>True on success</returns>
		/// ------------------------------------------------------------------------------------
		public void SaveGridProperties(DataGridView grid, string gridLinesValue)
		{
			if (grid == null)
				return;

			if (m_xmlDoc == null)
				CreateNewSettingsFile();

			XmlNode node = VerifyNodeExists(kGridsNode);
			if (node == null)
				return;

			RemoveChildNode(node, grid.Name);

			XmlElement gridElement = m_xmlDoc.CreateElement("grid");
			gridElement.SetAttribute("id", grid.Name);
			gridElement.SetAttribute("colheaderheight", grid.ColumnHeadersHeight.ToString());
			if (gridLinesValue != null)
				gridElement.SetAttribute("lines", gridLinesValue);
			
			node = node.AppendChild(gridElement);

			// Before saving the settings, we need to store the columns in a sorted list.
			// That way the columns can be saved in the display order. This is necessary
			// for loading them later. If they don't get saved in sorted order, the
			// display order's don't get loaded properly.
			SortedList<int, DataGridViewColumn> colList = new SortedList<int, DataGridViewColumn>();
			foreach (DataGridViewColumn col in grid.Columns)
			{
				if (!(col is SilHierarchicalGridColumn))
					colList[col.DisplayIndex] = col;
			}

			foreach (KeyValuePair<int, DataGridViewColumn> col in colList)
			{
				XmlElement colElement = m_xmlDoc.CreateElement("column");
				colElement.SetAttribute("id", col.Value.Name);
				colElement.SetAttribute("visible", col.Value.Visible.ToString());
				colElement.SetAttribute("width", col.Value.Width.ToString());
				colElement.SetAttribute("displayindex", col.Value.DisplayIndex.ToString());
				node.AppendChild(colElement);
			}

			m_xmlDoc.Save(m_settingsFile);
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Assigns properties to the specified grid.
		/// </summary>
		/// <param name="grid">Grid for whose settings are being loaded</param>
		/// <returns>True on success</returns>
		/// ------------------------------------------------------------------------------------
		public bool LoadGridProperties(DataGridView grid)
		{
			string gridLinesValue;
			return LoadGridProperties(grid, out gridLinesValue);
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Assigns properties to the specified grid.
		/// </summary>
		/// <param name="grid">Grid for whose settings are being loaded</param>
		/// <param name="gridLinesValue">string value corresponding to the type of grid
		/// lines read from the settings file for the specified grid</param>
		/// <returns>True on success</returns>
		/// ------------------------------------------------------------------------------------
		public bool LoadGridProperties(DataGridView grid, out string gridLinesValue)
		{
			gridLinesValue = null;

			if (m_xmlDoc == null || grid == null)
				return false;

			XmlNode node = FindChildNode(kGridsNode, grid.Name);

			if (node == null)
			    return false;

			// Get the cell border style.
			gridLinesValue = XMLHelper.GetAttributeValue(node, "lines");

			// Get the column header height. If it's not there, then auto. calculate it.
			int colHdrHeight = XMLHelper.GetIntFromAttribute(node, "colheaderheight", -1);
			if (colHdrHeight == -1)
				grid.AutoResizeColumnHeadersHeight();
			else
				grid.ColumnHeadersHeight = colHdrHeight;

			// Get each column's properties.
			node = node.FirstChild;
			while (node != null)
			{
				try
				{
					string id = XMLHelper.GetAttributeValue(node, "id");
					grid.Columns[id].Visible = XMLHelper.GetBoolFromAttribute(node, "visible", true);

					int width = XMLHelper.GetIntFromAttribute(node, "width", -1);
					if (width > -1)
						grid.Columns[id].Width = width;

					int displayIndex = XMLHelper.GetIntFromAttribute(node, "displayindex", -1);
					if (displayIndex > -1)
						grid.Columns[id].DisplayIndex = displayIndex;
				}
				catch {}

				node = node.NextSibling;
			}
			
			return true;
		}

		#endregion

		#region Methods for getting and setting generic values for any window.
		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Gets an integer value from the settings file.
		/// </summary>
		/// ------------------------------------------------------------------------------------
		public int GetIntSettingsValue(string settingName, string property, int defaultValue)
		{
			return GetIntValue(kMiscSettingsNode, settingName, property, defaultValue);
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Gets an float value from the settings file.
		/// </summary>
		/// ------------------------------------------------------------------------------------
		public float GetFloatSettingsValue(string settingName, string property, float defaultValue)
		{
			return GetFloatValue(kMiscSettingsNode, settingName, property, defaultValue);
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Gets a string value from the settings file.
		/// </summary>
		/// ------------------------------------------------------------------------------------
		public string GetStringSettingsValue(string settingName, string property, string defaultValue)
		{
			return GetStringValue(kMiscSettingsNode, settingName, property, defaultValue);
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Gets a boolean value from the settings file.
		/// </summary>
		/// ------------------------------------------------------------------------------------
		public bool GetBoolSettingsValue(string settingName, string property, bool defaultValue)
		{
			return GetBoolValue(kMiscSettingsNode, settingName, property, defaultValue);
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Gets a 8 digit hex string from the settings file and converts it to an ARGB value
		/// in order to return a color.
		/// </summary>
		/// ------------------------------------------------------------------------------------
		public Color GetColorSettingsValue(string settingName, string property, Color defaultValue)
		{
			return GetColorSettingsValue(settingName, property, (uint)defaultValue.ToArgb());
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Gets a 8 digit hex string from the settings file and converts it to an ARGB value
		/// in order to return a color.
		/// </summary>
		/// ------------------------------------------------------------------------------------
		public Color GetColorSettingsValue(string settingName, string property, uint defaultValue)
		{
			// Get the string value.
			string sClrVal = GetStringSettingsValue(settingName, property, null);

			try
			{
				return Color.FromArgb(int.Parse(sClrVal, System.Globalization.NumberStyles.AllowHexSpecifier));
			}
			catch { }

			try
			{
				return Color.FromArgb((int)defaultValue);
			}
			catch { }

			return Color.Black;
		}
		
		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Saves a value to the settings file for the specified window.
		/// </summary>
		/// <param name="settingName">Name of setting.</param>
		/// <param name="property">Name of the item being saved (this is used for the
		/// attribute name in the XML node). For example: "splitter1Location"</param>
		/// <param name="value">value being saved.</param>
		/// ------------------------------------------------------------------------------------
		public void SaveSettingsValue(string settingName, string property, object value)
		{
			if (value != null)
			{
				if (value.GetType() == typeof(Color))
					value = ((Color)value).ToArgb().ToString("X");

				SaveValue(kMiscSettingsNode, "setting", settingName, property, value);
			}
		}

		#endregion

		#region Generic methods for save/getting settings in a specified node block
		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Saves a value to the settings file for the specified window.
		/// </summary>
		/// <param name="xPath">xpath of settings block (e.g. "windowstates")</param>
		/// <param name="elementName">name of subelements within the specified xPath
		/// (e.g. window).</param>
		/// <param name="id">Unique id of settings record (e.g. window name).</param>
		/// <param name="property">Name of the item being saved (this is used for the
		/// attribute name in the XML node). For example: "splitter1Location"</param>
		/// <param name="value">value being saved.</param>
		/// ------------------------------------------------------------------------------------
		public void SaveValue(string xPath, string elementName, string id,
			string property, object value)
		{
			if (value == null)
				return;

			if (m_xmlDoc == null)
				CreateNewSettingsFile();

			XmlNode node = VerifyNodeExists(xPath);
			if (node == null)
				return;

			XmlNode childNode = FindChildNode(node, id);
			if (childNode == null)
			{
				XmlElement element = m_xmlDoc.CreateElement(elementName);
				element.SetAttribute("id", id);
				childNode = node.AppendChild(element);
			}

			childNode.Attributes.RemoveNamedItem(property);
			XmlAttribute attr = m_xmlDoc.CreateAttribute(property);
			attr.Value = value.ToString();
			childNode.Attributes.Append(attr);

			m_xmlDoc.Save(m_settingsFile);
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Utility method for the GetStringSettingsValue, GetIntSettingsValue and
		/// GetBoolSettingsValue methods.
		/// </summary>
		/// <param name="xPath">xpath of settings block (e.g. "windowstates")</param>
		/// <param name="id">Unique id of settings record (e.g. window name).</param>
		/// <param name="property">Name of the item whose value to retrieve (this is used for
		/// the attribute name in the XML node). For example: "splitter1Location"</param>
		/// <returns></returns>
		/// ------------------------------------------------------------------------------------
		private string GetValue(string xPath, string id, string property)
		{
			if (m_xmlDoc == null)
				return null;

			XmlNode node = FindChildNode(xPath, id);

			if (node == null)
				return null;

			return XMLHelper.GetAttributeValue(node, property);
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Gets an float value from the settings file.
		/// </summary>
		/// ------------------------------------------------------------------------------------
		public float GetFloatValue(string xPath, string id, string property, float defaultValue)
		{
			string strVal = GetValue(xPath, id, property);
			float floatVal;
			return (float.TryParse(strVal, out floatVal) ? floatVal : defaultValue);
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Gets an integer value from the settings file.
		/// </summary>
		/// ------------------------------------------------------------------------------------
		public int GetIntValue(string xPath, string id, string property, int defaultValue)
		{
			string strVal = GetValue(xPath, id, property);
			int intVal;
			return (int.TryParse(strVal, out intVal) ? intVal : defaultValue);
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Gets a string value from the settings file.
		/// </summary>
		/// ------------------------------------------------------------------------------------
		public string GetStringValue(string xPath, string id, string property, string defaultValue)
		{
			string retVal = GetValue(xPath, id, property);
			return (retVal == null ? defaultValue : retVal);
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Gets a boolean value from the settings file.
		/// </summary>
		/// ------------------------------------------------------------------------------------
		public bool GetBoolValue(string xPath, string id, string property, bool defaultValue)
		{
			string retVal = GetValue(xPath, id, property);
			try {return bool.Parse(retVal);}
			catch { }
			return defaultValue;
		}

		#endregion

		#region Helper Methods
		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Finds the child node (of the parent node specified xpath) whose id attribute's
		/// value is equal to the one specified.
		/// </summary>
		/// <param name="xpath"></param>
		/// <param name="id"></param>
		/// <returns></returns>
		/// ------------------------------------------------------------------------------------
		public XmlNode FindChildNode(string xpath, string id)
		{
			XmlNode parentNode = m_xmlDoc.SelectSingleNode(xpath);
			return FindChildNode(parentNode, id);
		}
		
		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Finds the child node (of the specified parent node) whose id attribute's value
		/// is equal to the one specified.
		/// </summary>
		/// <param name="parentNode"></param>
		/// <param name="id"></param>
		/// <returns></returns>
		/// ------------------------------------------------------------------------------------
		public XmlNode FindChildNode(XmlNode parentNode, string id)
		{
			if (parentNode != null)
			{
				XmlNode node = parentNode.FirstChild;

				while (node != null)
				{
					XmlAttribute attrib = node.Attributes["id"];
					if (attrib != null && attrib.Value == id)
						return node;

					node = node.NextSibling;
				}
			}

			return null;
		}
		
		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Removes the child node (of the specified parent node) whose id attribute's value
		/// is equal to the one specified.
		/// </summary>
		/// <param name="parentNode"></param>
		/// <param name="id"></param>
		/// ------------------------------------------------------------------------------------
		protected void RemoveChildNode(XmlNode parentNode, string id)
		{
			XmlNode node = FindChildNode(parentNode, id);
			if (node != null)
				parentNode.RemoveChild(node);
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Only works if xpath is unique
		/// </summary>
		/// <param name="xpath"></param>
		/// <param name="val"></param>
		/// ------------------------------------------------------------------------------------
		protected void SetStringValue(string xpath, string attribute, string val)
		{
			if (m_xmlDoc == null || val == null || val.Trim() == string.Empty)
				return;

			// Verify the node exists. If not, create it.
			XmlElement node = (XmlElement)VerifyNodeExists(xpath);
			
			if (node != null)
				node.SetAttribute(attribute, val);

			m_xmlDoc.Save(m_settingsFile);
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Sets an integer value to the settings file.
		/// </summary>
		/// ------------------------------------------------------------------------------------
		protected void SetIntValue(string xpath, string attribute, int val)
		{
			SetStringValue(xpath, attribute, val.ToString());
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// Sets an integer value to the settings file.
		/// </summary>
		/// ------------------------------------------------------------------------------------
		protected void SetFloatValue(string xpath, string attribute, float val)
		{
			SetStringValue(xpath, attribute, val.ToString());
		}

		/// ------------------------------------------------------------------------------------
		/// <summary>
		/// 
		/// </summary>
		/// ------------------------------------------------------------------------------------
		protected XmlNode VerifyNodeExists(string xpath)
		{
			xpath = xpath.TrimStart(new char[] {'/'});
			xpath = xpath.TrimEnd(new char[] {'/'});

			XmlNode node = m_xmlDoc.SelectSingleNode(xpath);
			
			// If the specified x-path is not valid, then pick off the last part of it
			// and see if that's valid. Then add a node to make the full x-path valid.
			if (node == null)
			{
				string childPath = xpath;
				int i = xpath.LastIndexOf("/");
				if (i > 0)
				{
					// Save what's to the right of the slash.
					childPath = xpath.Substring(i + 1);
					
					// Get the node associated with the path to the left of the slash.
					node = VerifyNodeExists(xpath.Substring(0, i + 1));
				}

				XmlElement element = m_xmlDoc.CreateElement(childPath);
				node.AppendChild(element);
				node = m_xmlDoc.SelectSingleNode(xpath);
				m_xmlDoc.Save(m_settingsFile);
			}
			
			return node;
		}

		#endregion
	}
}