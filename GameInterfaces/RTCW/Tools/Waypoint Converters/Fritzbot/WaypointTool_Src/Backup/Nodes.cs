﻿//------------------------------------------------------------------------------
// <autogenerated>
//     This code was generated by a tool.
//     Runtime Version: 1.1.4322.2300
//
//     Changes to this file may cause incorrect behavior and will be lost if 
//     the code is regenerated.
// </autogenerated>
//------------------------------------------------------------------------------

namespace WaypointTool {
    using System;
    using System.Data;
    using System.Xml;
    using System.Runtime.Serialization;
    
    
    [Serializable()]
    [System.ComponentModel.DesignerCategoryAttribute("code")]
    [System.Diagnostics.DebuggerStepThrough()]
    [System.ComponentModel.ToolboxItem(true)]
    public class Nodes : DataSet {
        
        private NodeDataTable tableNode;
        
        public Nodes() {
            this.InitClass();
            System.ComponentModel.CollectionChangeEventHandler schemaChangedHandler = new System.ComponentModel.CollectionChangeEventHandler(this.SchemaChanged);
            this.Tables.CollectionChanged += schemaChangedHandler;
            this.Relations.CollectionChanged += schemaChangedHandler;
        }
        
        protected Nodes(SerializationInfo info, StreamingContext context) {
            string strSchema = ((string)(info.GetValue("XmlSchema", typeof(string))));
            if ((strSchema != null)) {
                DataSet ds = new DataSet();
                ds.ReadXmlSchema(new XmlTextReader(new System.IO.StringReader(strSchema)));
                if ((ds.Tables["Node"] != null)) {
                    this.Tables.Add(new NodeDataTable(ds.Tables["Node"]));
                }
                this.DataSetName = ds.DataSetName;
                this.Prefix = ds.Prefix;
                this.Namespace = ds.Namespace;
                this.Locale = ds.Locale;
                this.CaseSensitive = ds.CaseSensitive;
                this.EnforceConstraints = ds.EnforceConstraints;
                this.Merge(ds, false, System.Data.MissingSchemaAction.Add);
                this.InitVars();
            }
            else {
                this.InitClass();
            }
            this.GetSerializationData(info, context);
            System.ComponentModel.CollectionChangeEventHandler schemaChangedHandler = new System.ComponentModel.CollectionChangeEventHandler(this.SchemaChanged);
            this.Tables.CollectionChanged += schemaChangedHandler;
            this.Relations.CollectionChanged += schemaChangedHandler;
        }
        
        [System.ComponentModel.Browsable(false)]
        [System.ComponentModel.DesignerSerializationVisibilityAttribute(System.ComponentModel.DesignerSerializationVisibility.Content)]
        public NodeDataTable Node {
            get {
                return this.tableNode;
            }
        }
        
        public override DataSet Clone() {
            Nodes cln = ((Nodes)(base.Clone()));
            cln.InitVars();
            return cln;
        }
        
        protected override bool ShouldSerializeTables() {
            return false;
        }
        
        protected override bool ShouldSerializeRelations() {
            return false;
        }
        
        protected override void ReadXmlSerializable(XmlReader reader) {
            this.Reset();
            DataSet ds = new DataSet();
            ds.ReadXml(reader);
            if ((ds.Tables["Node"] != null)) {
                this.Tables.Add(new NodeDataTable(ds.Tables["Node"]));
            }
            this.DataSetName = ds.DataSetName;
            this.Prefix = ds.Prefix;
            this.Namespace = ds.Namespace;
            this.Locale = ds.Locale;
            this.CaseSensitive = ds.CaseSensitive;
            this.EnforceConstraints = ds.EnforceConstraints;
            this.Merge(ds, false, System.Data.MissingSchemaAction.Add);
            this.InitVars();
        }
        
        protected override System.Xml.Schema.XmlSchema GetSchemaSerializable() {
            System.IO.MemoryStream stream = new System.IO.MemoryStream();
            this.WriteXmlSchema(new XmlTextWriter(stream, null));
            stream.Position = 0;
            return System.Xml.Schema.XmlSchema.Read(new XmlTextReader(stream), null);
        }
        
        internal void InitVars() {
            this.tableNode = ((NodeDataTable)(this.Tables["Node"]));
            if ((this.tableNode != null)) {
                this.tableNode.InitVars();
            }
        }
        
        private void InitClass() {
            this.DataSetName = "Nodes";
            this.Prefix = "";
            this.Namespace = "http://tempuri.org/Nodes.xsd";
            this.Locale = new System.Globalization.CultureInfo("en-US");
            this.CaseSensitive = false;
            this.EnforceConstraints = true;
            this.tableNode = new NodeDataTable();
            this.Tables.Add(this.tableNode);
        }
        
        private bool ShouldSerializeNode() {
            return false;
        }
        
        private void SchemaChanged(object sender, System.ComponentModel.CollectionChangeEventArgs e) {
            if ((e.Action == System.ComponentModel.CollectionChangeAction.Remove)) {
                this.InitVars();
            }
        }
        
        public delegate void NodeRowChangeEventHandler(object sender, NodeRowChangeEvent e);
        
        [System.Diagnostics.DebuggerStepThrough()]
        public class NodeDataTable : DataTable, System.Collections.IEnumerable {
            
            private DataColumn columnID;
            
            private DataColumn columnFlag;
            
            private DataColumn columnRadius;
            
            private DataColumn columnEntity;
            
            private DataColumn columnTeam;
            
            private DataColumn columnGroup;
            
            private DataColumn columnConnection1;
            
            private DataColumn columnConnection2;
            
            private DataColumn columnConnection3;
            
            private DataColumn columnConnection4;
            
            internal NodeDataTable() : 
                    base("Node") {
                this.InitClass();
            }
            
            internal NodeDataTable(DataTable table) : 
                    base(table.TableName) {
                if ((table.CaseSensitive != table.DataSet.CaseSensitive)) {
                    this.CaseSensitive = table.CaseSensitive;
                }
                if ((table.Locale.ToString() != table.DataSet.Locale.ToString())) {
                    this.Locale = table.Locale;
                }
                if ((table.Namespace != table.DataSet.Namespace)) {
                    this.Namespace = table.Namespace;
                }
                this.Prefix = table.Prefix;
                this.MinimumCapacity = table.MinimumCapacity;
                this.DisplayExpression = table.DisplayExpression;
            }
            
            [System.ComponentModel.Browsable(false)]
            public int Count {
                get {
                    return this.Rows.Count;
                }
            }
            
            internal DataColumn IDColumn {
                get {
                    return this.columnID;
                }
            }
            
            internal DataColumn FlagColumn {
                get {
                    return this.columnFlag;
                }
            }
            
            internal DataColumn RadiusColumn {
                get {
                    return this.columnRadius;
                }
            }
            
            internal DataColumn EntityColumn {
                get {
                    return this.columnEntity;
                }
            }
            
            internal DataColumn TeamColumn {
                get {
                    return this.columnTeam;
                }
            }
            
            internal DataColumn GroupColumn {
                get {
                    return this.columnGroup;
                }
            }
            
            internal DataColumn Connection1Column {
                get {
                    return this.columnConnection1;
                }
            }
            
            internal DataColumn Connection2Column {
                get {
                    return this.columnConnection2;
                }
            }
            
            internal DataColumn Connection3Column {
                get {
                    return this.columnConnection3;
                }
            }
            
            internal DataColumn Connection4Column {
                get {
                    return this.columnConnection4;
                }
            }
            
            public NodeRow this[int index] {
                get {
                    return ((NodeRow)(this.Rows[index]));
                }
            }
            
            public event NodeRowChangeEventHandler NodeRowChanged;
            
            public event NodeRowChangeEventHandler NodeRowChanging;
            
            public event NodeRowChangeEventHandler NodeRowDeleted;
            
            public event NodeRowChangeEventHandler NodeRowDeleting;
            
            public void AddNodeRow(NodeRow row) {
                this.Rows.Add(row);
            }
            
            public NodeRow AddNodeRow(string ID, string Flag, string Radius, string Entity, string Team, string Group, string Connection1, string Connection2, string Connection3, string Connection4) {
                NodeRow rowNodeRow = ((NodeRow)(this.NewRow()));
                rowNodeRow.ItemArray = new object[] {
                        ID,
                        Flag,
                        Radius,
                        Entity,
                        Team,
                        Group,
                        Connection1,
                        Connection2,
                        Connection3,
                        Connection4};
                this.Rows.Add(rowNodeRow);
                return rowNodeRow;
            }
            
            public System.Collections.IEnumerator GetEnumerator() {
                return this.Rows.GetEnumerator();
            }
            
            public override DataTable Clone() {
                NodeDataTable cln = ((NodeDataTable)(base.Clone()));
                cln.InitVars();
                return cln;
            }
            
            protected override DataTable CreateInstance() {
                return new NodeDataTable();
            }
            
            internal void InitVars() {
                this.columnID = this.Columns["ID"];
                this.columnFlag = this.Columns["Flag"];
                this.columnRadius = this.Columns["Radius"];
                this.columnEntity = this.Columns["Entity"];
                this.columnTeam = this.Columns["Team"];
                this.columnGroup = this.Columns["Group"];
                this.columnConnection1 = this.Columns["Connection1"];
                this.columnConnection2 = this.Columns["Connection2"];
                this.columnConnection3 = this.Columns["Connection3"];
                this.columnConnection4 = this.Columns["Connection4"];
            }
            
            private void InitClass() {
                this.columnID = new DataColumn("ID", typeof(string), null, System.Data.MappingType.Element);
                this.Columns.Add(this.columnID);
                this.columnFlag = new DataColumn("Flag", typeof(string), null, System.Data.MappingType.Element);
                this.Columns.Add(this.columnFlag);
                this.columnRadius = new DataColumn("Radius", typeof(string), null, System.Data.MappingType.Element);
                this.Columns.Add(this.columnRadius);
                this.columnEntity = new DataColumn("Entity", typeof(string), null, System.Data.MappingType.Element);
                this.Columns.Add(this.columnEntity);
                this.columnTeam = new DataColumn("Team", typeof(string), null, System.Data.MappingType.Element);
                this.Columns.Add(this.columnTeam);
                this.columnGroup = new DataColumn("Group", typeof(string), null, System.Data.MappingType.Element);
                this.Columns.Add(this.columnGroup);
                this.columnConnection1 = new DataColumn("Connection1", typeof(string), null, System.Data.MappingType.Element);
                this.Columns.Add(this.columnConnection1);
                this.columnConnection2 = new DataColumn("Connection2", typeof(string), null, System.Data.MappingType.Element);
                this.Columns.Add(this.columnConnection2);
                this.columnConnection3 = new DataColumn("Connection3", typeof(string), null, System.Data.MappingType.Element);
                this.Columns.Add(this.columnConnection3);
                this.columnConnection4 = new DataColumn("Connection4", typeof(string), null, System.Data.MappingType.Element);
                this.Columns.Add(this.columnConnection4);
            }
            
            public NodeRow NewNodeRow() {
                return ((NodeRow)(this.NewRow()));
            }
            
            protected override DataRow NewRowFromBuilder(DataRowBuilder builder) {
                return new NodeRow(builder);
            }
            
            protected override System.Type GetRowType() {
                return typeof(NodeRow);
            }
            
            protected override void OnRowChanged(DataRowChangeEventArgs e) {
                base.OnRowChanged(e);
                if ((this.NodeRowChanged != null)) {
                    this.NodeRowChanged(this, new NodeRowChangeEvent(((NodeRow)(e.Row)), e.Action));
                }
            }
            
            protected override void OnRowChanging(DataRowChangeEventArgs e) {
                base.OnRowChanging(e);
                if ((this.NodeRowChanging != null)) {
                    this.NodeRowChanging(this, new NodeRowChangeEvent(((NodeRow)(e.Row)), e.Action));
                }
            }
            
            protected override void OnRowDeleted(DataRowChangeEventArgs e) {
                base.OnRowDeleted(e);
                if ((this.NodeRowDeleted != null)) {
                    this.NodeRowDeleted(this, new NodeRowChangeEvent(((NodeRow)(e.Row)), e.Action));
                }
            }
            
            protected override void OnRowDeleting(DataRowChangeEventArgs e) {
                base.OnRowDeleting(e);
                if ((this.NodeRowDeleting != null)) {
                    this.NodeRowDeleting(this, new NodeRowChangeEvent(((NodeRow)(e.Row)), e.Action));
                }
            }
            
            public void RemoveNodeRow(NodeRow row) {
                this.Rows.Remove(row);
            }
        }
        
        [System.Diagnostics.DebuggerStepThrough()]
        public class NodeRow : DataRow {
            
            private NodeDataTable tableNode;
            
            internal NodeRow(DataRowBuilder rb) : 
                    base(rb) {
                this.tableNode = ((NodeDataTable)(this.Table));
            }
            
            public string ID {
                get {
                    try {
                        return ((string)(this[this.tableNode.IDColumn]));
                    }
                    catch (InvalidCastException e) {
                        throw new StrongTypingException("Cannot get value because it is DBNull.", e);
                    }
                }
                set {
                    this[this.tableNode.IDColumn] = value;
                }
            }
            
            public string Flag {
                get {
                    try {
                        return ((string)(this[this.tableNode.FlagColumn]));
                    }
                    catch (InvalidCastException e) {
                        throw new StrongTypingException("Cannot get value because it is DBNull.", e);
                    }
                }
                set {
                    this[this.tableNode.FlagColumn] = value;
                }
            }
            
            public string Radius {
                get {
                    try {
                        return ((string)(this[this.tableNode.RadiusColumn]));
                    }
                    catch (InvalidCastException e) {
                        throw new StrongTypingException("Cannot get value because it is DBNull.", e);
                    }
                }
                set {
                    this[this.tableNode.RadiusColumn] = value;
                }
            }
            
            public string Entity {
                get {
                    try {
                        return ((string)(this[this.tableNode.EntityColumn]));
                    }
                    catch (InvalidCastException e) {
                        throw new StrongTypingException("Cannot get value because it is DBNull.", e);
                    }
                }
                set {
                    this[this.tableNode.EntityColumn] = value;
                }
            }
            
            public string Team {
                get {
                    try {
                        return ((string)(this[this.tableNode.TeamColumn]));
                    }
                    catch (InvalidCastException e) {
                        throw new StrongTypingException("Cannot get value because it is DBNull.", e);
                    }
                }
                set {
                    this[this.tableNode.TeamColumn] = value;
                }
            }
            
            public string Group {
                get {
                    try {
                        return ((string)(this[this.tableNode.GroupColumn]));
                    }
                    catch (InvalidCastException e) {
                        throw new StrongTypingException("Cannot get value because it is DBNull.", e);
                    }
                }
                set {
                    this[this.tableNode.GroupColumn] = value;
                }
            }
            
            public string Connection1 {
                get {
                    try {
                        return ((string)(this[this.tableNode.Connection1Column]));
                    }
                    catch (InvalidCastException e) {
                        throw new StrongTypingException("Cannot get value because it is DBNull.", e);
                    }
                }
                set {
                    this[this.tableNode.Connection1Column] = value;
                }
            }
            
            public string Connection2 {
                get {
                    try {
                        return ((string)(this[this.tableNode.Connection2Column]));
                    }
                    catch (InvalidCastException e) {
                        throw new StrongTypingException("Cannot get value because it is DBNull.", e);
                    }
                }
                set {
                    this[this.tableNode.Connection2Column] = value;
                }
            }
            
            public string Connection3 {
                get {
                    try {
                        return ((string)(this[this.tableNode.Connection3Column]));
                    }
                    catch (InvalidCastException e) {
                        throw new StrongTypingException("Cannot get value because it is DBNull.", e);
                    }
                }
                set {
                    this[this.tableNode.Connection3Column] = value;
                }
            }
            
            public string Connection4 {
                get {
                    try {
                        return ((string)(this[this.tableNode.Connection4Column]));
                    }
                    catch (InvalidCastException e) {
                        throw new StrongTypingException("Cannot get value because it is DBNull.", e);
                    }
                }
                set {
                    this[this.tableNode.Connection4Column] = value;
                }
            }
            
            public bool IsIDNull() {
                return this.IsNull(this.tableNode.IDColumn);
            }
            
            public void SetIDNull() {
                this[this.tableNode.IDColumn] = System.Convert.DBNull;
            }
            
            public bool IsFlagNull() {
                return this.IsNull(this.tableNode.FlagColumn);
            }
            
            public void SetFlagNull() {
                this[this.tableNode.FlagColumn] = System.Convert.DBNull;
            }
            
            public bool IsRadiusNull() {
                return this.IsNull(this.tableNode.RadiusColumn);
            }
            
            public void SetRadiusNull() {
                this[this.tableNode.RadiusColumn] = System.Convert.DBNull;
            }
            
            public bool IsEntityNull() {
                return this.IsNull(this.tableNode.EntityColumn);
            }
            
            public void SetEntityNull() {
                this[this.tableNode.EntityColumn] = System.Convert.DBNull;
            }
            
            public bool IsTeamNull() {
                return this.IsNull(this.tableNode.TeamColumn);
            }
            
            public void SetTeamNull() {
                this[this.tableNode.TeamColumn] = System.Convert.DBNull;
            }
            
            public bool IsGroupNull() {
                return this.IsNull(this.tableNode.GroupColumn);
            }
            
            public void SetGroupNull() {
                this[this.tableNode.GroupColumn] = System.Convert.DBNull;
            }
            
            public bool IsConnection1Null() {
                return this.IsNull(this.tableNode.Connection1Column);
            }
            
            public void SetConnection1Null() {
                this[this.tableNode.Connection1Column] = System.Convert.DBNull;
            }
            
            public bool IsConnection2Null() {
                return this.IsNull(this.tableNode.Connection2Column);
            }
            
            public void SetConnection2Null() {
                this[this.tableNode.Connection2Column] = System.Convert.DBNull;
            }
            
            public bool IsConnection3Null() {
                return this.IsNull(this.tableNode.Connection3Column);
            }
            
            public void SetConnection3Null() {
                this[this.tableNode.Connection3Column] = System.Convert.DBNull;
            }
            
            public bool IsConnection4Null() {
                return this.IsNull(this.tableNode.Connection4Column);
            }
            
            public void SetConnection4Null() {
                this[this.tableNode.Connection4Column] = System.Convert.DBNull;
            }
        }
        
        [System.Diagnostics.DebuggerStepThrough()]
        public class NodeRowChangeEvent : EventArgs {
            
            private NodeRow eventRow;
            
            private DataRowAction eventAction;
            
            public NodeRowChangeEvent(NodeRow row, DataRowAction action) {
                this.eventRow = row;
                this.eventAction = action;
            }
            
            public NodeRow Row {
                get {
                    return this.eventRow;
                }
            }
            
            public DataRowAction Action {
                get {
                    return this.eventAction;
                }
            }
        }
    }
}
