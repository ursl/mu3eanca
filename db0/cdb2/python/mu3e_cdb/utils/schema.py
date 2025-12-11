"""Schema parser for CDB payloads.

Parses schema strings and uses them to decode BLOBs dynamically.
Schema format: "ui_id,ui_ckdivend,i_ncol[,i_col,ui_qual],..."
- ui_ prefix = unsigned int
- i_ prefix = int
- b_ prefix = bool
- v/d prefix or no prefix = double
- [field1,field2,...] = optional/repeated fields (count comes before the bracket)
"""

import re
from typing import List, Dict, Optional, Any, Iterator
from enum import Enum


class FieldType(Enum):
    """Field type enumeration."""
    UINT = "uint"
    INT = "int"
    BOOL = "bool"
    DOUBLE = "double"


class SchemaField:
    """Represents a single field in the schema."""
    
    def __init__(self, name: str, field_type: FieldType, is_repeated: bool = False):
        self.name = name
        self.type = field_type
        self.is_repeated = is_repeated
        self.repeated_fields: Optional[List['SchemaField']] = None  # For [field1,field2] patterns
    
    def __repr__(self) -> str:
        if self.is_repeated:
            return f"SchemaField({self.name}, {self.type.value}, repeated={self.repeated_fields})"
        return f"SchemaField({self.name}, {self.type.value})"


class SchemaParser:
    """Parser for CDB schema strings."""
    
    @staticmethod
    def parse(schema: str) -> List[SchemaField]:
        """
        Parse a schema string into a list of SchemaField objects.
        
        Args:
            schema: Schema string like "ui_id,ui_ckdivend,i_ncol[,i_col,ui_qual]"
        
        Returns:
            List of SchemaField objects
        """
        if not schema:
            return []
        
        fields = []
        # Remove whitespace
        schema = schema.replace(" ", "")
        
        # Split by comma, but preserve brackets
        tokens = SchemaParser._tokenize(schema)
        
        i = 0
        while i < len(tokens):
            token = tokens[i]
            
            if token == "[":
                # Start of optional/repeated fields
                # The previous field should be a count field
                if not fields:
                    raise ValueError(f"Unexpected '[' at position {i}: no preceding count field")
                
                # Find matching closing bracket
                bracket_count = 1
                j = i + 1
                repeated_tokens = []
                while j < len(tokens) and bracket_count > 0:
                    if tokens[j] == "[":
                        bracket_count += 1
                    elif tokens[j] == "]":
                        bracket_count -= 1
                        if bracket_count == 0:
                            break
                    repeated_tokens.append(tokens[j])
                    j += 1
                
                if bracket_count != 0:
                    raise ValueError(f"Unmatched '[' starting at position {i}")
                
                # Parse repeated fields
                repeated_fields = SchemaParser._parse_field_list(repeated_tokens)
                
                # Mark the last field as repeated
                fields[-1].is_repeated = True
                fields[-1].repeated_fields = repeated_fields
                
                i = j + 1
            else:
                # Regular field
                field_type = SchemaParser._infer_type(token)
                fields.append(SchemaField(token, field_type))
                i += 1
        
        return fields
    
    @staticmethod
    def _tokenize(schema: str) -> List[str]:
        """Tokenize schema string, preserving brackets."""
        tokens = []
        current = ""
        
        for char in schema:
            if char in "[]":
                if current:
                    tokens.append(current)
                    current = ""
                tokens.append(char)
            elif char == ",":
                # Comma is a separator, not a token
                if current:
                    tokens.append(current)
                    current = ""
            else:
                current += char
        
        if current:
            tokens.append(current)
        
        return tokens
    
    @staticmethod
    def _parse_field_list(tokens: List[str]) -> List[SchemaField]:
        """Parse a list of field tokens into SchemaField objects."""
        fields = []
        for token in tokens:
            if token in "[],":
                continue
            field_type = SchemaParser._infer_type(token)
            fields.append(SchemaField(token, field_type))
        return fields
    
    @staticmethod
    def _infer_type(name: str) -> FieldType:
        """Infer field type from name prefix."""
        if name.startswith("ui_"):
            return FieldType.UINT
        elif name.startswith("i_"):
            return FieldType.INT
        elif name.startswith("b_"):
            return FieldType.BOOL
        else:
            return FieldType.DOUBLE


class SchemaDecoder:
    """Decodes BLOBs using a parsed schema."""
    
    def __init__(self, schema: str):
        self.schema_str = schema
        self.fields = SchemaParser.parse(schema)
    
    def decode_blob(self, blob_binary: bytes, blob_iter: Iterator[int]) -> Dict[str, Any]:
        """
        Decode a BLOB using the schema.
        
        Args:
            blob_binary: Binary BLOB data (for reference)
            blob_iter: Iterator over bytes
        
        Returns:
            Dictionary with decoded values (empty dict if no data available)
        
        Raises:
            StopIteration: If not enough data to read even the first field
        """
        from .blob import get_data
        
        result = {}
        
        for field in self.fields:
            # Strip prefix from field name for easier access
            field_key = self.strip_prefix(field.name)
            
            if field.is_repeated:
                # Read count first
                try:
                    count_data = get_data(blob_iter)
                    count = self._read_value(count_data, field.type)
                    result[field_key] = count
                    
                    # Read repeated fields if count > 0
                    if count > 0 and field.repeated_fields:
                        repeated_values = []
                        for i in range(count):
                            item = {}
                            for repeated_field in field.repeated_fields:
                                data = get_data(blob_iter)
                                value = self._read_value(data, repeated_field.type)
                                # Strip prefix from repeated field names too
                                repeated_key = self.strip_prefix(repeated_field.name)
                                item[repeated_key] = value
                            # Add complete item (all repeated fields read successfully)
                            repeated_values.append(item)
                        result[f"{field_key}_data"] = repeated_values
                except StopIteration:
                    # Not enough data for count field - end of record
                    raise
            else:
                try:
                    data = get_data(blob_iter)
                    value = self._read_value(data, field.type)
                    result[field_key] = value
                except StopIteration:
                    # Not enough data - end of record
                    raise
        
        return result
    
    def _read_value(self, data: bytes, field_type: FieldType) -> Any:
        """Read a value from binary data based on field type."""
        from .blob import blob2uint, blob2int, blob2double
        
        if field_type == FieldType.UINT:
            return blob2uint(data)
        elif field_type == FieldType.INT:
            return blob2int(data)
        elif field_type == FieldType.BOOL:
            return bool(blob2uint(data))
        elif field_type == FieldType.DOUBLE:
            return blob2double(data)
        else:
            raise ValueError(f"Unknown field type: {field_type}")
    
    @staticmethod
    def strip_prefix(name: str) -> str:
        """Strip type prefix from field name (ui_, i_, b_, etc.)."""
        if name.startswith("ui_"):
            return name[3:]
        elif name.startswith("i_"):
            return name[2:]
        elif name.startswith("b_"):
            return name[2:]
        return name

